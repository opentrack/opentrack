/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "spline.hpp"

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

using namespace spline_detail;

constexpr std::size_t spline::value_count;

spline::spline(const QString& name, const QString& axis_name, Axis axis) :
    axis(axis)
{
    set_bundle(options::make_bundle(name), axis_name, axis);
}

spline::~spline()
{
    QMutexLocker l(&_mutex);

    if (connection)
    {
        QObject::disconnect(connection);
        QObject::disconnect(conn_maxx);
        QObject::disconnect(conn_maxy);
        connection = QMetaObject::Connection();
        conn_maxx = QMetaObject::Connection();
        conn_maxy = QMetaObject::Connection();
    }
}

spline::spline() : spline("", "", Axis(-1)) {}

void spline::set_tracking_active(bool value)
{
    QMutexLocker l(&_mutex);
    activep = value;
}

bundle spline::get_bundle()
{
    QMutexLocker l(&_mutex); // avoid logic errors due to changes in value<t> data
    return s->b;
}

void spline::clear()
{
    QMutexLocker l(&_mutex);
    s->points = points_t();
    validp = false;
}

float spline::get_value(double x)
{
    QMutexLocker foo(&_mutex);

    const float ret = get_value_no_save(x);
    last_input_value.setX(std::fabs(x));
    last_input_value.setY(double(std::fabs(ret)));
    return ret;
}

float spline::get_value_no_save(double x) const
{
    return const_cast<spline&>(*this).get_value_no_save_internal(x);
}

float spline::get_value_no_save_internal(double x)
{
    QMutexLocker foo(&_mutex);

    float  q  = float(x * bucket_size_coefficient(s->points));
    int    xi = (int)q;
    float  yi = get_value_internal(xi);
    float  yiplus1 = get_value_internal(xi+1);
    float  f = (q-xi);
    float  ret = yiplus1 * f + yi * (1.0f - f); // at least do a linear interpolation.
    return ret;
}

warn_result_unused bool spline::get_last_value(QPointF& point)
{
    QMutexLocker foo(&_mutex);
    point = last_input_value;
    return activep;
}

template <typename T>
static T signum(T val)
{
    return (T(0) < val) - (val < T(0));
}

float spline::get_value_internal(int x)
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

void spline::add_lone_point()
{
    points_t points;
    points.push_back(QPointF(s->opts.clamp_x_, s->opts.clamp_y_));

    s->points = points;
}

QPointF spline::ensure_in_bounds(const QList<QPointF>& points, int i)
{
    const int sz = points.size();

    if (i < 0 || sz == 0)
        return QPointF(0, 0);

    if (i < sz)
        return points[i];

    return points[sz - 1];
}

int spline::element_count(const QList<QPointF>& points, double max_input)
{
    const unsigned sz = points.size();
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

void spline::update_interp_data()
{
    points_t points = s->points;
    ensure_valid(points);
    const int sz = points.size();

    const double maxx = max_input();

    if (sz == 0)
        points.prepend(QPointF(maxx, max_output()));

    std::stable_sort(points.begin(), points.begin() + sz, sort_fn);

    const double c = bucket_size_coefficient(points);
    const double c_interp = c * 30;

    for (unsigned i = 0; i < value_count; i++)
        data[i] = -16;

    if (sz < 2)
    {
        if (points[0].x() - 1e-2 < maxx)
        {
            const double x = points[0].x();
            const double y = points[0].y();
            const int max = clamp(iround(x * c), 1, value_count-1);
            for (int k = 0; k <= max; k++)
            {
                if (k < value_count)
                    data[unsigned(k)] = float(y * k / max);
            }
        }
    }
    else
    {
        if (points[0].x() > 1e-2 && points[0].x() <= maxx)
            points.push_front(QPointF(0, 0));

        for (int i = 0; i < sz; i++)
        {
            const QPointF p0 = ensure_in_bounds(points, i - 1);
            const QPointF p1 = ensure_in_bounds(points, i + 0);
            const QPointF p2 = ensure_in_bounds(points, i + 1);
            const QPointF p3 = ensure_in_bounds(points, i + 2);
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
            const unsigned end = int(c_interp * (p2_x - p1_x)) + 1;

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

    float last = 0;
    for (unsigned i = 0; i < unsigned(value_count); i++)
    {
        if (data[i] == -16)
            data[i] = last;
        last = data[i];
    }
}

void spline::remove_point(int i)
{
    QMutexLocker foo(&_mutex);

    points_t points = s->points;
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
    QMutexLocker foo(&_mutex);

    points_t points = s->points;
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
    QMutexLocker foo(&_mutex);

    points_t points = s->points;

    const int sz = element_count(points, max_input());

    if (idx >= 0 && idx < sz)
    {
        points[idx] = pt;
        // we don't allow points to be reordered, but sort due to possible caller logic error
        std::stable_sort(points.begin(), points.end(), sort_fn);
        s->points = points;
        validp = false;
    }
}

spline::points_t spline::get_points() const
{
    QMutexLocker foo(&_mutex);
    return s->points;
}

int spline::get_point_count() const
{
    QMutexLocker foo(&_mutex);
    return element_count(s->points, s->opts.clamp_x_);
}

void spline::reload()
{
    QMutexLocker foo(&_mutex);
    s->b->reload();
}

void spline::save()
{
    QMutexLocker foo(&_mutex);
    s->b->save();
}

void spline::invalidate_settings()
{
    // we're holding the mutex to allow signal disconnection in spline dtor
    // before this slot gets called for the next time

    QMutexLocker l(&_mutex);
    validp = false;

    emit s->recomputed();
}

void spline::set_bundle(bundle b, const QString& axis_name, Axis axis)
{
    QMutexLocker l(&_mutex);

    // gets called from ctor hence the need for nullptr checks
    // the sentinel settings/bundle objects don't need any further branching once created
    if (!s || s->b != b)
    {
        s = std::make_shared<settings>(b, axis_name, axis);

        if (connection)
        {
            QObject::disconnect(connection);
            QObject::disconnect(conn_maxx);
            QObject::disconnect(conn_maxy);
        }

        if (b)
        {
            connection = QObject::connect(b.get(), &bundle_::changed,
                                          s.get(), [&]() { invalidate_settings(); });

            // this isn't strictly necessary for the spline but helps the widget
            conn_maxx = QObject::connect(&s->opts.clamp_x_, base_value::value_changed<int>(),
                                         ctx.get(), [&](double) { invalidate_settings(); });
            conn_maxy = QObject::connect(&s->opts.clamp_y_, base_value::value_changed<int>(),
                                         ctx.get(), [&](double) { invalidate_settings(); });
        }

        validp = false;
    }
}

double spline::max_input() const
{
    QMutexLocker l(&_mutex);
    if (s)
    {
        using m = axis_opts::max_clamp;
        const value<m>& clamp = s->opts.clamp_x_;
        const QList<QPointF> points = s->points;
        if (clamp == m::x1000 && points.size())
            return points[points.size() - 1].x();
        return s ? std::fabs(clamp.to<double>()) : 0;
    }
    return 0;
}

double spline::max_output() const
{
    QMutexLocker l(&_mutex);
    if (s)
    {
        using m = axis_opts::max_clamp;
        const value<m>& clamp = s->opts.clamp_y_;
        const QList<QPointF> points = s->points;
        if (clamp == m::x1000 && points.size())
            return points[points.size() - 1].y();
        return s ? std::fabs(clamp.to<double>()) : 0;
    }
    return 0;
}

void spline::ensure_valid(QList<QPointF>& the_points)
{
    QMutexLocker foo(&_mutex);

    QList<QPointF> list = the_points;

    // storing to s->points fires bundle::changed and that leads to an infinite loop
    // thus, only store if we can't help it
    std::stable_sort(list.begin(), list.end(), sort_fn);

    const int sz = list.size();

    QList<QPointF> ret_list, ret_list_2;
    ret_list.reserve(sz), ret_list_2.reserve(sz);

    const double maxx = max_input(), maxy = max_output();

    for (int i = 0; i < sz; i++)
    {
        QPointF& pt(list[i]);

        const bool overlap = progn(
            for (int j = 0; j < i; j++)
            {
                const QPointF& pt2(list[j]);
                const QPointF tmp(pt - pt2);
                const double dist_sq = QPointF::dotProduct(tmp, tmp);
                const double overlap = maxx / 500.;
                if (dist_sq < overlap * overlap)
                    return true;
            }
            return false;
        );

        if (!overlap)
            ret_list_2.append(pt);

        if (pt.x() - 1e-2 < maxx && pt.x() >= 0 &&
            pt.y() - 1e-2 < maxy && pt.y() >= 0 && !overlap)
        {
            ret_list.push_back(pt);
        }
    }

    if (ret_list != the_points)
    {
        s->points = std::move(ret_list_2);
        the_points = std::move(ret_list);
    }

    last_input_value = QPointF(0, 0);
    activep = false;
}

// the return value is only safe to use with no spline::set_bundle calls
std::shared_ptr<base_settings> spline::get_settings()
{
    QMutexLocker foo(&_mutex);
    return std::static_pointer_cast<base_settings>(s);
}

std::shared_ptr<const base_settings> spline::get_settings() const
{
    QMutexLocker foo(&_mutex);
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

namespace spline_detail {

settings::settings(bundle b, const QString& axis_name, Axis idx):
    b(b ? b : make_bundle("")),
    points(b, "points", {}),
    opts(axis_name, idx)
{}

settings::~settings()
{
}

}

