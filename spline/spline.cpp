#include "spline.hpp"
#include "compat/math.hpp"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <memory>
#include <cinttypes>
#include <numbers>
#include <utility>

#include <QObject>
#include <QMutexLocker>
#include <QPointF>
#include <QSettings>
#include <QString>

namespace spline_detail {

namespace {

// real roots of a·t³ + b·t² + c·t + d = 0
unsigned solve_cubic(double a, double b, double c, double d, double roots[3])
{
    // relative threshold for degree reduction; a vanishing leading
    // coefficient makes the normalized coefficients blow up
    constexpr double eps = 1e-9;
    const double scale = std::max({ std::fabs(a), std::fabs(b), std::fabs(c) });

    if (!(scale > 1e-12))
        return 0; // constant

    if (std::fabs(a) <= eps * scale)
    {
        if (std::fabs(b) <= eps * scale) // linear
        {
            roots[0] = -d / c;
            return 1;
        }
        const double disc = c * c - 4 * b * d; // quadratic
        if (disc < 0)
            return 0;
        const double q = -.5 * (c + std::copysign(std::sqrt(disc), c));
        roots[0] = q / b;
        roots[1] = q != 0 ? d / q : -c / b - roots[0];
        return 2;
    }

    // depressed cubic u³ + pu + q, t = u - B/3
    const double B = b / a, C = c / a, D = d / a;
    const double ofs = B / 3;
    const double p = C - B * B / 3;
    const double q = B * (2 * B * B - 9 * C) / 27 + D;
    const double disc = q * q / 4 + p * p * p / 27;

    if (disc > 0) // one real root (Cardano)
    {
        const double s = std::sqrt(disc);
        roots[0] = std::cbrt(-q / 2 + s) + std::cbrt(-q / 2 - s) - ofs;
        return 1;
    }
    if (p >= 0) // disc ≤ 0 forces p ≈ 0 here: triple root
    {
        roots[0] = std::cbrt(-q) - ofs;
        return 1;
    }
    // three real roots (viète's trigonometric method)
    const double m = 2 * std::sqrt(-p / 3);
    const double theta = std::acos(std::clamp(3 * q / (p * m), -1., 1.)) / 3;
    for (unsigned k = 0; k < 3; k++)
        roots[k] = m * std::cos(theta - 2 * std::numbers::pi * k / 3) - ofs;
    return 3;
}

// largest t ∈ [0, 1] with x(t) = X on a catmull-rom segment given its
// polynomial coefficients (same layout as `cx' below), or -1 if none.
// x(0) and x(1) are the segment endpoints, so for X between them a root
// exists; the largest one is taken so that for a non-monotonic x(t) the
// later part of the curve wins, like the overwriting sampler did.
double solve_segment_time(const double cx[4], double X)
{
    double roots[3];
    const unsigned n = solve_cubic(cx[3] / 2, cx[2] / 2, cx[1] / 2, cx[0] / 2 - X, roots);
    constexpr double eps = 1e-4;
    double t = -1;
    for (unsigned i = 0; i < n; i++)
        if (roots[i] >= -eps && roots[i] <= 1 + eps)
            t = std::max(t, std::clamp(roots[i], 0., 1.));
    return t;
}

} // anonymous ns

settings::~settings() = default;
base_spline_::~base_spline_() = default;
base_spline::~base_spline() = default;
spline_modify_mixin::~spline_modify_mixin() = default;
spline_settings_mixin::~spline_settings_mixin() = default;

spline::spline(const QString& name, const QString& axis_name, Axis axis)
{
    set_bundle(options::make_bundle(name), axis_name, axis);
}

spline::~spline()
{
    QMutexLocker l(&mtx);
    disconnect_signals();
}

spline::spline() : spline(QString{}, QString{}, Axis(-1)) {}

void spline::set_tracking_active(bool value) const
{
    std::shared_ptr<settings> S;
    {
        QMutexLocker l(&mtx);
        if (value == activep)
            return;
        activep = value;
        S = s;
    }
    emit S->recomputed();
}

bundle spline::get_bundle()
{
    QMutexLocker l(&mtx);
    return s->b;
}

void spline::clear()
{
    std::shared_ptr<settings> S;
    {
        QMutexLocker l(&mtx);
        S = s;
        s->points = {};
        points = {};
        update_interp_data();
    }
    emit S->recomputed();
}

double spline::get_value(double x) const
{
    QMutexLocker l(&mtx);

    const double ret = get_value_no_save(x);
    last_input_value = { std::fabs(x), std::fabs((double)ret) };
    return ret;
}

double spline::get_value_no_save(double x) const
{
    QMutexLocker l(&mtx);

    double q  = x * bucket_size_coefficient(points);
    int    xi = (int)q;
    double yi = get_value_internal(xi);
    double yiplus1 = get_value_internal(xi+1);
    double f = (q-xi);
    double ret = yiplus1 * f + yi * (1 - f); // at least do a linear interpolation.
    return ret;
}

bool spline::get_last_value(QPointF& point)
{
    QMutexLocker foo(&mtx);
    point = last_input_value;
    return activep && point.y() >= 0;
}

double spline::get_value_internal(int x) const
{
    const auto sign = (f)signum(x);
    x = std::abs(x);
    const auto ret_ = data[std::min(unsigned(x), value_count - 1)];
    return (double)(sign * std::clamp(ret_, (f)0, (f)1000));
}

void spline::ensure_in_bounds(const QList<QPointF>& points, int i, f& x, f& y)
{
    const int sz = points.size();

    if (i < 0 || sz == 0)
    {
        x = 0;
        y = 0;
    }
    else if (i < sz)
    {
        const QPointF& pt = points[i];
        x = (f)pt.x();
        y = (f)pt.y();
    }
    else
    {
        const QPointF& pt = points[sz - 1];
        x = (f)pt.x();
        y = (f)pt.y();
    }
}

int spline::element_count(const QList<QPointF>& points, double max_input)
{
    const int sz = points.size();
    for (int k = sz-1; k >= 0; k--)
    {
        const QPointF& pt = points[k];
        if (max_input > 1e-4 && pt.x() - 1e-4 >= max_input)
            return k;
    }
    return sz;
}

bool spline::sort_fn(QPointF one, QPointF two)
{
    return one.x() < two.x();
}

void spline::update_interp_data() const
{
    points_t list = points;
    ensure_valid(list);
    int sz = list.size();

    if (list.isEmpty())
        list.prepend({ max_input(), max_output() });

    const double c = bucket_size_coefficient(list);
    const f cf = (f)c;

    std::fill(data.begin(), data.end(), (f)0);

    if (sz < 2) // lerp only
    {
        const QPointF& pt = list[0];
        const double x = pt.x();
        const double y = pt.y();
        const unsigned max = std::clamp((unsigned)iround(x * c), 1u, value_count-1);

        for (unsigned k = 0; k <= max; k++)
            data[k] = f(y * k / max); // no need for bresenham
        for (unsigned k = max + 1; k < value_count; k++)
            data[k] = (f)y; // flat extension past the last point
    }
    else
    {
        if (list[0].x() > 1e-6)
        {
            double zero_pos = 0;
            while (list.size() > 1 && list[0].y() <= 1e-6)
            {
                zero_pos = list[0].x();
                list.pop_front();
            }
            list.push_front({zero_pos, 0});
            sz = list.size();
        }

        for (int i = 0; i + 1 < sz; i++)
        {
            f p0_x, p1_x, p2_x, p3_x;
            f p0_y, p1_y, p2_y, p3_y;

            ensure_in_bounds(list, i - 1, p0_x, p0_y);
            ensure_in_bounds(list, i + 0, p1_x, p1_y);
            ensure_in_bounds(list, i + 1, p2_x, p2_y);
            ensure_in_bounds(list, i + 2, p3_x, p3_y);

            const f cx[4] = {
                2 * p1_x, // 1
                -p0_x + p2_x, // t
                2 * p0_x - 5 * p1_x + 4 * p2_x - p3_x, // t^2
                -p0_x + 3 * p1_x - 3 * p2_x + p3_x, // t3
            };

            const f cy[4] = {
                2 * p1_y, // 1
                -p0_y + p2_y, // t
                2 * p0_y - 5 * p1_y + 4 * p2_y - p3_y, // t^2
                -p0_y + 3 * p1_y - 3 * p2_y + p3_y, // t3
            };

            // `get_value_no_save' reads data[k] as the value at x = k/c
            // exactly, so evaluate the curve per bucket: solve x(t) = k/c
            // for t in closed form, then take y(t)
            const unsigned start = (unsigned)std::ceil(p1_x * cf);
            const unsigned end = std::min(value_count - 1, (unsigned)std::floor(p2_x * cf));

            for (unsigned k = start; k <= end; k++)
            {
                const f X = cf > 0 ? std::clamp((f)k / cf, p1_x, p2_x) : p2_x;
                f t = (f)solve_segment_time(cx, X);
                if (t < 0) // fp trouble only; a root in [0, 1] must exist
                    t = p2_x - p1_x > 1e-9 ? std::clamp((X - p1_x) / (p2_x - p1_x), (f)0, (f)1) : (f)1;
                const f t2 = t*t;
                const f t3 = t2*t;
                const f y = f(.5) * (cy[0] + cy[1] * t + cy[2] * t2 + cy[3] * t3);

                if (std::isfinite(y))
                    data[k] = y;
            }
        }

        // flat extension past the last point, for when `clamp_x' rounds
        // the last point down to an interior bucket
        const unsigned tail = std::min(value_count - 1, (unsigned)std::floor(list[sz - 1].x() * cf));
        for (unsigned k = tail + 1; k < value_count; k++)
            data[k] = data[tail];
    }

    const auto maxy = (f)max_output();

    for (unsigned i = 0; i < value_count; i++)
        data[i] = std::clamp(data[i], (f)0, maxy);

    // make sure empty places stay empty (see #1341)
    if (auto it = std::find_if(list.cbegin(), list.cend(),
                               [](QPointF x) { return x.x() >= (f)1e-6 && x.y() >= (f)1e-6; });
        it != list.cend() && it != list.cbegin())
    {
        it--;
        unsigned max = std::clamp((unsigned)iround(it->x() * c), 0u, value_count-1);

        for (unsigned x = 0; x < max; x++)
            data[x] = 0;
    }
}

void spline::remove_point(int i)
{
    std::shared_ptr<settings> S;
    {
        QMutexLocker foo(&mtx);
        S = s;

        const int sz = element_count(points, max_input());

        if (i >= 0 && i < sz)
        {
            points.erase(points.begin() + i);
            s->points = points;
            update_interp_data();
        }
    }

    emit S->recomputed();
}

void spline::add_point(QPointF pt)
{
    std::shared_ptr<settings> S;
    {
        QMutexLocker foo(&mtx);
        S = s;
        points.push_back(pt);
        std::stable_sort(points.begin(), points.end(), sort_fn);
        s->points = points;
        update_interp_data();
    }
    emit S->recomputed();
}

void spline::add_point(double x, double y)
{
    add_point({ x, y });
}

void spline::move_point(int idx, QPointF pt)
{
    std::shared_ptr<settings> S;
    {
        QMutexLocker foo(&mtx);
        S = s;

        const int sz = element_count(points, max_input());

        if (idx >= 0 && idx < sz)
        {
            points[idx] = pt;
            std::stable_sort(points.begin(), points.end(), sort_fn);
            s->points = points;
            update_interp_data();
        }
    }
    emit S->recomputed();
}

const points_t& spline::get_points() const
{
    return points;
}

int spline::get_point_count() const
{
    QMutexLocker foo(&mtx);
    return element_count(points, clamp_x);
}

void spline::reload()
{
    QMutexLocker foo(&mtx);
    s->b->reload();
}

void spline::save()
{
    QMutexLocker foo(&mtx);
    s->b->save();
}

void spline::invalidate_settings_()
{
    points = s->points;
    clamp_x = s->opts.clamp_x_;
    clamp_y = s->opts.clamp_y_;
    update_interp_data();
}

void spline::invalidate_settings()
{
    std::shared_ptr<settings> S;
    {
        QMutexLocker l(&mtx);
        S = s;
        invalidate_settings_();
    }
    emit S->recomputed();
}

void spline::set_bundle(bundle b, const QString& axis_name, Axis axis)
{
    if (!b)
        b = make_bundle({});

    std::shared_ptr<settings> S;

    {
        QMutexLocker l(&mtx);

        disconnect_signals();
        s = std::make_shared<settings>(b, axis_name, axis);
        invalidate_settings_();
        S = s;

        conn_points = QObject::connect(&s->points, value_::value_changed<QList<QPointF>>(),
                                       &*ctx, [this] { invalidate_settings(); }, Qt::DirectConnection);
        conn_maxx   = QObject::connect(&s->opts.clamp_x_, value_::value_changed<int>(),
                                       &*ctx, [this](double) { invalidate_settings(); }, Qt::DirectConnection);
        conn_maxy   = QObject::connect(&s->opts.clamp_y_, value_::value_changed<int>(),
                                       &*ctx, [this](double) { invalidate_settings(); }, Qt::DirectConnection);
    }

    emit S->recomputed();
}

double spline::max_input() const
{
    QMutexLocker l(&mtx);
    if (clamp_x == axis_opts::x1000)
        return std::fmax(1, points.empty() ? 0 : points[points.size() - 1].x());
    return std::fabs((double)clamp_x);
}

double spline::max_output() const
{
    QMutexLocker l(&mtx);
    if (clamp_y == axis_opts::x1000 && !points.empty())
        return std::fmax(1, points.empty() ? 0 : points[points.size() - 1].y());
    return std::fabs((double)clamp_y);
}

void spline::ensure_valid(points_t& list) const
{
    std::stable_sort(list.begin(), list.end(), sort_fn);

    const unsigned sz = (unsigned)list.size();

    QList<QPointF> tmp_points, all_points;
    tmp_points.reserve(sz); all_points.reserve(sz);

    const double maxx = max_input();

    for (unsigned i = 0; i < sz; i++)
    {
        QPointF& pt{list[i]};

        bool overlap = false;
        for (unsigned j = i+1; j < sz; j++)
        {
            const QPointF& pt2{list[j]};
            const QPointF tmp(pt - pt2);
            const double dist_sq = QPointF::dotProduct(tmp, tmp);
            constexpr double min_dist = 1e-4;
            if (dist_sq < min_dist)
            {
                overlap = true;
                break;
            }
        }

        if (!overlap)
        {
            all_points.append(pt); // all points total

            // points within selected limit, for use in `update_interp_data'
            if (pt.x() - 1e-4 <= maxx && pt.x() >= 0)
                tmp_points.push_back(pt);
        }
    }

    // simply storing to s->points fires bundle::changed leading to a livelock
    // hence only store if we can't help it

    if (all_points.size() < points.size())
    {
        // all points that don't overlap
        points = std::move(all_points);
        s->points = points;
    }

    if (tmp_points.size() < list.size())
        // points that are within currently-specified bounds
        list = std::move(tmp_points);

    last_input_value = {};
    activep = false;
}

std::shared_ptr<base_settings> spline::get_settings()
{
    QMutexLocker foo(&mtx);
    return std::static_pointer_cast<base_settings>(s);
}

std::shared_ptr<const base_settings> spline::get_settings() const
{
    QMutexLocker foo(&mtx);
    return std::static_pointer_cast<const base_settings>(s);
}

double spline::bucket_size_coefficient(const QList<QPointF>& points) const
{
    constexpr double eps = 1e-4;
    const double maxx = max_input();

    if (maxx < eps)
        return 0;

    // buckets are only used between 0 and the rightmost point's
    // x coordinate. even if `clamp_x' is set to a much larger value,
    // buckets retain more precision lower the x coordinate of the
    // last point.

    const int sz = element_count(points, maxx);
    const double last_x = sz ? points[sz - 1].x() : maxx;

    return std::clamp((value_count-1) / std::clamp(last_x, eps, maxx), 0., (value_count-1.));
}

void spline::disconnect_signals()
{
    if (conn_points)
    {
        QObject::disconnect(conn_points); conn_points = {};
        QObject::disconnect(conn_maxx); conn_maxx = {};
        QObject::disconnect(conn_maxy); conn_maxy = {};
    }
}

settings::settings(bundle const& b, const QString& axis_name, Axis idx):
    b(b ? b : make_bundle({})),
    opts(axis_name, idx)
{}

} // ns spline_detail
