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

#include <QObject>
#include <QMutexLocker>
#include <QCoreApplication>
#include <QPointF>
#include <QSettings>
#include <QString>

#include <QDebug>

constexpr int spline::value_count;

spline::spline(qreal maxx, qreal maxy, const QString& name) :
    s(nullptr),
    data(value_count, -16),
    _mutex(QMutex::Recursive),
    max_x(maxx),
    max_y(maxy),
    activep(false),
    validp(false)
{
    set_bundle(options::make_bundle(name));
}

spline::~spline()
{
    QMutexLocker l(&_mutex);

    if (connection)
    {
        QObject::disconnect(connection);
        connection = QMetaObject::Connection();
    }
}

spline::spline() : spline(0, 0, "") {}

void spline::set_tracking_active(bool value)
{
    QMutexLocker l(&_mutex);
    activep = value;
}

bundle spline::get_bundle()
{
    return s->b;
}

void spline::clear()
{
    QMutexLocker l(&_mutex);
    s->points = points_t();
    validp = false;
}

void spline::set_max_input(qreal max_input)
{
    QMutexLocker l(&_mutex);
    max_x = max_input;
    validp = false;
}

void spline::set_max_output(qreal max_output)
{
    QMutexLocker l(&_mutex);
    max_y = max_output;
    validp = false;
}

qreal spline::max_input() const
{
    QMutexLocker l(&_mutex);
    return max_x;
}

qreal spline::max_output() const
{
    QMutexLocker l(&_mutex);
    return max_y;
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

    if (max_x > 0)
        x = std::fmin(max_x, x);

    float  q  = float(x * bucket_size_coefficient(s->points));
    int    xi = (int)q;
    float  yi = get_value_internal(xi);
    float  yiplus1 = get_value_internal(xi+1);
    float  f = (q-xi);
    float  ret = yiplus1 * f + yi * (1.0f - f); // at least do a linear interpolation.
    return ret;
}

DEFUN_WARN_UNUSED bool spline::get_last_value(QPointF& point)
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
    float ret = sign * std::fmax(0, ret_);
    if (max_y > 0)
        ret = fmin(max_y, ret);
    return ret;
}

void spline::add_lone_point()
{
    points_t points;
    points.push_back(QPointF(max_x, max_y));

    s->points = points;
}

QPointF spline::ensure_in_bounds(const QList<QPointF>& points, double max_x, int i)
{
    const int sz = element_count(points, max_x);

    if (i < 0 || sz == 0)
        return QPointF(0, 0);

    if (i < sz)
        return points[i];

    return points[sz - 1];
}

int spline::element_count(const QList<QPointF>& points, double max_x)
{
    if (!(max_x > 0))
        return points.size();
    else
    {
        const unsigned sz = points.size();
        for (unsigned i = 0; i < sz; i++)
        {
            if (!(points[i].x() <= max_x))
                return i;
        }
        return points.size();
    }
}

bool spline::sort_fn(const QPointF& one, const QPointF& two)
{
    return one.x() < two.x();
}

void spline::update_interp_data()
{
    points_t points = s->points;

    ensure_valid(points);

    int sz = element_count(points, max_x);

    if (sz == 0)
        points.prepend(QPointF(max_x, max_y));

    std::stable_sort(points.begin(), points.begin() + sz, sort_fn);

    const double c = bucket_size_coefficient(points);
    const double c_interp = c * 30;

    for (unsigned i = 0; i < value_count; i++)
        data[i] = -16;

    if (sz < 2)
    {
        if (points[0].x() - 1e-2 <= max_x)
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
        if (points[0].x() > 1e-2 && points[0].x() <= max_x)
            points.push_front(QPointF(0, 0));

        for (int i = 0; i < sz; i++)
        {
            const QPointF p0 = ensure_in_bounds(points, max_x, i - 1);
            const QPointF p1 = ensure_in_bounds(points, max_x, i + 0);
            const QPointF p2 = ensure_in_bounds(points, max_x, i + 1);
            const QPointF p3 = ensure_in_bounds(points, max_x, i + 2);
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

                if (x >= 0 && x < value_count)
                {
                    data[x] = y;
                }
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
    const int sz = element_count(points, max_x);

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

    const int sz = element_count(points, max_x);

    if (idx >= 0 && idx < sz)
    {
        points[idx] = pt;
        // we don't allow points to be reordered, but sort due to possible caller logic error
        std::stable_sort(points.begin(), points.end(), sort_fn);
        s->points = points;
        validp = false;
    }
}

QList<QPointF> spline::get_points() const
{
    QMutexLocker foo(&_mutex);
    return s->points;
}

int spline::get_point_count() const
{
    QMutexLocker foo(&_mutex);
    return element_count(s->points, max_x);
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

void spline::set_bundle(bundle b)
{
    QMutexLocker l(&_mutex);

    // gets called from ctor hence the need for nullptr checks
    // the sentinel settings/bundle objects don't need any further branching once created
    if (!s || s->b != b)
    {
        s = std::make_shared<settings>(b);

        if (connection)
            QObject::disconnect(connection);

        if (b)
        {
            connection = QObject::connect(b.get(), &bundle_::changed,
            s.get(), [&]()
            {
                    // we're holding the mutex to allow signal disconnection in spline dtor
                    // before this slot gets called for the next time

                    // spline isn't a QObject and the connection context is incorrect

                    QMutexLocker l(&_mutex);
                    validp = false;

                    emit s->recomputed();
                },
            Qt::QueuedConnection);
        }

        validp = false;
    }
}

void spline::ensure_valid(const QList<QPointF>& the_points)
{
    QMutexLocker foo(&_mutex);

    QList<QPointF> list = the_points;

    // storing to s->points fires bundle::changed and that leads to an infinite loop
    // thus, only store if we can't help it
    std::stable_sort(list.begin(), list.end(), sort_fn);

    const int sz = list.size();

    QList<QPointF> ret_list;
    ret_list.reserve(sz);

    for (int i = 0; i < sz; i++)
    {
        QPointF& pt(list[i]);

        const bool overlap = progn(
            for (int j = 0; j < i; j++)
            {
                const QPointF& pt2(list[j]);
                const QPointF tmp(pt - pt2);
                const double dist_sq = QPointF::dotProduct(tmp, tmp);
                const double overlap = max_x / 500.;
                if (dist_sq < overlap * overlap)
                    return true;
            }
            return false;
        );
        if (!overlap)
            ret_list.push_back(pt);
    }

    if (ret_list != the_points)
        s->points = ret_list;

    last_input_value = QPointF(0, 0);
    activep = false;
}

// the return value is only safe to use with no spline::set_bundle calls
std::shared_ptr<spline::settings> spline::get_settings()
{
    QMutexLocker foo(&_mutex);
    return s;
}

std::shared_ptr<const spline::settings> spline::get_settings() const
{
    QMutexLocker foo(&_mutex);
    return s;
}

double spline::bucket_size_coefficient(const QList<QPointF>& points) const
{
    static constexpr double eps = 1e-4;

    if (unlikely(max_x < eps))
        return 0;

    // needed to fill the buckets up to the last control point.
    // space between that point and max_x doesn't matter.

    const int sz = element_count(points, max_x);
    const double last_x = sz ? points[sz - 1].x() : max_x;

    return clamp((value_count-1) / clamp(last_x, eps, max_x), 0., (value_count-1));
}

namespace spline_detail {

settings::settings(bundle b):
    b(b ? b : make_bundle("")),
    points(b, "points", QList<QPointF>())
{}

settings::~settings()
{
}

}
