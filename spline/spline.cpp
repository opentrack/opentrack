/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "spline.hpp"
#include "compat/math.hpp"

#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <memory>
#include <cinttypes>
#include <utility>

#include <QObject>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QPointF>
#include <QSettings>
#include <QString>

#include <QDebug>

namespace spline_detail {

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

void spline::set_tracking_active(bool value)
{
    QMutexLocker l(&mtx);
    activep = value;
}

bundle spline::get_bundle()
{
    QMutexLocker l(&mtx); // avoid logic errors due to changes in value<t> data
    return s->b;
}

void spline::clear()
{
    {
        QMutexLocker l(&mtx);
        s->points = {};
    }

    invalidate_settings();
}

float spline::get_value(double x)
{
    QMutexLocker foo(&mtx);

    const float ret = get_value_no_save(x);
    last_input_value.setX(std::fabs(x));
    last_input_value.setY(double(std::fabs(ret)));
    return ret;
}

float spline::get_value_no_save(double x) const
{
    QMutexLocker foo(&mtx);

    float  q  = float(x * bucket_size_coefficient(points));
    int    xi = (int)q;
    float  yi = get_value_internal(xi);
    float  yiplus1 = get_value_internal(xi+1);
    float  f = (q-xi);
    float  ret = yiplus1 * f + yi * (1.0f - f); // at least do a linear interpolation.
    return ret;
}

bool spline::get_last_value(QPointF& point)
{
    QMutexLocker foo(&mtx);
    point = last_input_value;
    return activep;
}

float spline::get_value_internal(int x) const
{
    if (!validp)
    {
        update_interp_data();
        validp = true;
    }

    const float sign = signum(x);
    x = std::abs(x);
    const float ret_ = data[std::min(unsigned(x), unsigned(value_count)-1u)];
    return sign * clamp(ret_, 0, 1000);
}

QPointF spline::ensure_in_bounds(const QList<QPointF>& points, int i)
{
    const int sz = points.size();

    if (i < 0 || sz == 0)
        return {};

    if (i < sz)
        return points[i];

    return points[sz - 1];
}

int spline::element_count(const QList<QPointF>& points, double max_input)
{
    const unsigned sz = (unsigned)points.size();
    for (unsigned k = 0; k < sz; k++)
    {
        const QPointF& pt = points[k];
        if (max_input > 1e-4 && pt.x() - 1e-2 > max_input)
            return k;
    }
    return sz;
}

bool spline::sort_fn(const QPointF& one, const QPointF& two)
{
    return one.x() < two.x();
}

void spline::update_interp_data() const
{
    points_t list = points;
    ensure_valid(list);
    const int sz = list.size();

    const double maxx = max_input();

    if (list.isEmpty())
        list.prepend({ maxx, max_output() });

    const double c = bucket_size_coefficient(list);
    const double c_interp = c * 30;

    enum { magic_fill_value = -255 };

    for (unsigned i = 0; i < value_count; i++)
        data[i] = magic_fill_value;

    if (sz < 2) // lerp only
    {
        const QPointF& pt = list[0];
        const double x = pt.x();
        const double y = pt.y();
        const unsigned max = clamp(uround(x * c), 0, value_count-2);

        for (unsigned k = 0; k <= max; k++)
            data[k] = float(y * k / max); // no need for bresenham
    }
    else
    {
        if (list[0].x() > 1e-2 && list[0].x() <= maxx)
            list.push_front(QPointF(0, 0));

        // now this is hella expensive due to `c_interp'
        for (int i = 0; i < sz; i++)
        {
            const QPointF p0 = ensure_in_bounds(list, i - 1);
            const QPointF p1 = ensure_in_bounds(list, i + 0);
            const QPointF p2 = ensure_in_bounds(list, i + 1);
            const QPointF p3 = ensure_in_bounds(list, i + 2);
            const double p0_x = p0.x(), p1_x = p1.x(), p2_x = p2.x(), p3_x = p3.x();
            const double p0_y = p0.y(), p1_y = p1.y(), p2_y = p2.y(), p3_y = p3.y();

            const double cx[4] = {
                2 * p1_x, // 1
                -p0_x + p2_x, // t
                2 * p0_x - 5 * p1_x + 4 * p2_x - p3_x, // t^2
                -p0_x + 3 * p1_x - 3 * p2_x + p3_x, // t3
            };

            const double cy[4] =
            {
                2 * p1_y, // 1
                -p0_y + p2_y, // t
                2 * p0_y - 5 * p1_y + 4 * p2_y - p3_y, // t^2
                -p0_y + 3 * p1_y - 3 * p2_y + p3_y, // t3
            };

            // multiplier helps fill in all the x's needed
            const unsigned end{int(c_interp * (p2_x - p1_x)) + 1u};

            for (unsigned k = 0; k <= end; k++)
            {
                const double t = k / double(end);
                const double t2 = t*t;
                const double t3 = t*t*t;

                const int x = int(.5 * c * (cx[0] + cx[1] * t + cx[2] * t2 + cx[3] * t3));
                const float y = float(.5 * (cy[0] + cy[1] * t + cy[2] * t2 + cy[3] * t3));

                if (unsigned(x) < value_count)
                    data[x] = y;
            }
        }
    }

    double maxy = max_output();
    float last = 0;

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wfloat-equal"
#endif

    for (unsigned i = 0; i < unsigned(value_count); i++)
    {
        if (data[i] == magic_fill_value)
            data[i] = last;
        data[i] = clamp(data[i], 0, (float)maxy);
        last = data[i];
    }

#ifdef __clang__
#   pragma clang diagnostic pop
#endif
}

void spline::remove_point(int i)
{
    QMutexLocker foo(&mtx);

    const int sz = element_count(points, max_input());

    if (i >= 0 && i < sz)
    {
        points.erase(points.begin() + i);
        s->points = points;
        validp = false;
    }
}

void spline::add_point(QPointF pt)
{
    QMutexLocker foo(&mtx);

    points.push_back(pt);
    std::stable_sort(points.begin(), points.end(), sort_fn);
    s->points = points;
    validp = false;
}

void spline::add_point(double x, double y)
{
    add_point(QPointF(x, y));
}

void spline::move_point(int idx, QPointF pt)
{
    QMutexLocker foo(&mtx);

    const int sz = element_count(points, max_input());

    if (idx >= 0 && idx < sz)
    {
        points[idx] = pt;
        std::stable_sort(points.begin(), points.end(), sort_fn);
        s->points = points;
        validp = false;
    }
}

const base_spline_::points_t& spline::get_points() const
{
    QMutexLocker foo(&mtx);
    return points;
}

int spline::get_point_count() const
{
    QMutexLocker foo(&mtx);
    return element_count(points, s->opts.clamp_x_);
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

void spline::invalidate_settings()
{
    {
        QMutexLocker l(&mtx);
        validp = false;
        points = s->points;
    }
    emit s->recomputed();
}

void spline::set_bundle(bundle b, const QString& axis_name, Axis axis)
{
    QMutexLocker l(&mtx);

    // gets called from ctor hence the need for nullptr checks
    // the sentinel settings/bundle objects don't need any further branching once created
    if (!s || s->b != b)
    {
        disconnect_signals();

        if (!b)
            b = make_bundle(QString{});
        s = std::make_shared<settings>(b, axis_name, axis);

        conn_changed = QObject::connect(b.get(), &bundle_::changed,
                                        s.get(), [&] { invalidate_settings(); }, Qt::QueuedConnection);
        // this isn't strictly necessary for the spline but helps the widget
        conn_maxx = QObject::connect(&s->opts.clamp_x_, value_::value_changed<int>(),
                                     ctx.get(), [&](double) { invalidate_settings(); }, Qt::QueuedConnection);
        conn_maxy = QObject::connect(&s->opts.clamp_y_, value_::value_changed<int>(),
                                     ctx.get(), [&](double) { invalidate_settings(); }, Qt::QueuedConnection);

        invalidate_settings();
    }
}

double spline::max_input() const
{
    QMutexLocker l(&mtx);
    using m = axis_opts::max_clamp;
    const value<m>& clamp = s->opts.clamp_x_;
    if (clamp == m::x1000 && !points.empty())
        return points[points.size() - 1].x();
    return std::fabs(clamp.to<double>());
}

double spline::max_output() const
{
    QMutexLocker l(&mtx);
    using m = axis_opts::max_clamp;
    const value<m>& clamp = s->opts.clamp_y_;
    if (clamp == m::x1000 && !points.empty())
        return points[points.size() - 1].y();
    return std::fabs(clamp.to<double>());
}

void spline::ensure_valid(points_t& list) const
{
    QMutexLocker foo(&mtx);

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

    // needed to fill the buckets up to the last control point.
    // space between that point and max_x doesn't matter.

    const int sz = element_count(points, maxx);
    const double last_x = sz ? points[sz - 1].x() : maxx;

    return clamp((value_count-1) / clamp(last_x, eps, maxx), 0., (value_count-1));
}

void spline::disconnect_signals()
{
    if (conn_changed)
    {
        QObject::disconnect(conn_changed); conn_changed = {};
        QObject::disconnect(conn_maxx); conn_maxx = {};
        QObject::disconnect(conn_maxy); conn_maxy = {};
    }
}

settings::settings(bundle const& b, const QString& axis_name, Axis idx):
    b(b ? b : make_bundle("")),
    opts(axis_name, idx)
{}

settings::~settings() = default;

} // ns spline_detail
