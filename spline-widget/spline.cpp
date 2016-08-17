/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "spline.hpp"
#include <QMutexLocker>
#include <QCoreApplication>
#include <QPointF>
#include <QSettings>
#include <QString>
#include <algorithm>
#include <cmath>

#include <QDebug>

constexpr int spline::value_count;

void spline::setTrackingActive(bool blnActive)
{
    activep = blnActive;
}

spline::spline() : spline(0, 0, "")
{
}

void spline::removeAllPoints()
{
    QMutexLocker l(&_mutex);
    s->points = points_t();
    update_interp_data();
}

void spline::setMaxInput(qreal max_input)
{
    QMutexLocker l(&_mutex);
    max_x = max_input;
}

void spline::setMaxOutput(qreal max_output)
{
    QMutexLocker l(&_mutex);
    max_y = max_output;
}

qreal spline::maxInput() const
{
    QMutexLocker l(&_mutex);
    return max_x;
}

qreal spline::maxOutput() const
{
    QMutexLocker l(&_mutex);
    return max_y;
}

// todo try non-recursive mtx
spline::spline(qreal maxx, qreal maxy, const QString& name) :
    data(value_count, -1.f),
    _mutex(QMutex::Recursive),
    max_x(maxx),
    max_y(maxy),
    activep(false)
{
    set_bundle(options::make_bundle(name));
}

float spline::getValue(double x)
{
    QMutexLocker foo(&_mutex);
    float q  = float(x * precision(s->points));
    int    xi = (int)q;
    float  yi = getValueInternal(xi);
    float  yiplus1 = getValueInternal(xi+1);
    float  f = (q-xi);
    float  ret = yiplus1 * f + yi * (1.0f - f); // at least do a linear interpolation.
    last_input_value.setX(std::fabs(x));
    last_input_value.setY(double(std::fabs(ret)));
    return ret;
}

bool spline::getLastPoint(QPointF& point )
{
    QMutexLocker foo(&_mutex);
    point = last_input_value;
    return activep;
}

float spline::getValueInternal(int x)
{
    float sign = x < 0 ? -1 : 1;
    x = abs(x);
    float ret;
        ret = data[std::min(unsigned(x), unsigned(value_count)-1u)];
    return ret * sign;
}

void spline::add_lone_point()
{
    points_t points;
    points.push_back(QPointF(max_x, max_y));

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

bool spline::sort_fn(const QPointF& one, const QPointF& two)
{
    return one.x() < two.x();
}

void spline::update_interp_data()
{
    points_t points = s->points;

    if (points.size() == 0)
        points.append(QPointF(max_x, max_y));

    std::stable_sort(points.begin(), points.end(), sort_fn);

    const double mult = precision(points);
    const double mult_ = mult * 30;

    for (unsigned i = 0; i < value_count; i++)
        data[i] = -1;

    if (points.size() == 1)
    {
        const double x = points[0].x();
        const double y = points[0].y();
        const int max = clamp(int(x * precision(points)), 0, value_count-1);
        for (int k = 0; k <= max; k++)
        {
            if (k < value_count)
                data[unsigned(k)] = float(y * k / max);
        }
    }
    else
    {
        if (points[0].x() > 1e-2)
            points.push_front(QPointF(0, 0));

        for (int i = 0; i < points.size(); i++)
        {
            const QPointF p0 = ensure_in_bounds(points, i - 1);
            const QPointF p1 = ensure_in_bounds(points, i);
            const QPointF p2 = ensure_in_bounds(points, i + 1);
            const QPointF p3 = ensure_in_bounds(points, i + 2);

            const double p0_x = p0.x(), p1_x = p1.x(), p2_x = p2.x(), p3_x = p3.x();
            const double p0_y = p0.y(), p1_y = p1.y(), p2_y = p2.y(), p3_y = p3.y();

            // multiplier helps fill in all the x's needed
            const unsigned end = std::min(unsigned(value_count), unsigned(p2_x * mult_));
            const unsigned start = std::max(0u, unsigned(p1_x * mult));

            for (unsigned j = start; j < end; j++)
            {
                const double t = (j - start) / (double) (end - start);
                const double t2 = t*t;
                const double t3 = t*t*t;

                const int x = int(.5 * ((2 * p1_x) +
                                        (-p0_x + p2_x) * t +
                                        (2 * p0_x - 5 * p1_x + 4 * p2_x - p3_x) * t2 +
                                        (-p0_x + 3 * p1_x - 3 * p2_x + p3_x) * t3)
                                  * mult);

                const float y = float(.5 * ((2 * p1_y) +
                                            (-p0_y + p2_y) * t +
                                            (2 * p0_y - 5 * p1_y + 4 * p2_y - p3_y) * t2 +
                                            (-p0_y + 3 * p1_y - 3 * p2_y + p3_y) * t3));

                if (x >= 0 && x < value_count)
                    data[unsigned(x)] = y;
            }
        }
    }

    float last = 0;
    for (unsigned i = 0; i < unsigned(value_count); i++)
    {
        if (data[i] < 0)
            data[i] = last;
        last = data[i];
    }
}

void spline::removePoint(int i)
{
    QMutexLocker foo(&_mutex);

    points_t points = s->points;

    if (i >= 0 && i < points.size())
    {
        points.erase(points.begin() + i);
        s->points = points;
        update_interp_data();
    }
}

void spline::addPoint(QPointF pt)
{
    QMutexLocker foo(&_mutex);

    points_t points = s->points;
    points.push_back(pt);
    std::stable_sort(points.begin(), points.end(), sort_fn);
    s->points = points;
    update_interp_data();
}

void spline::movePoint(int idx, QPointF pt)
{
    QMutexLocker foo(&_mutex);

    points_t points = s->points;

    if (idx >= 0 && idx < points.size())
    {
        points[idx] = pt;
        // we don't allow points to be reordered, but sort due to possible caller logic error
        std::stable_sort(points.begin(), points.end(), sort_fn);
        s->points = points;
        update_interp_data();
    }
}

QList<QPointF> spline::getPoints() const
{
    QMutexLocker foo(&_mutex);
    return s->points;
}

void spline::reload()
{
    if (s && s->b)
        s->b->reload();
}

void spline::save(QSettings& settings)
{
    if (s && s->b)
        s->b->save_deferred(settings);
}

void spline::save()
{
    save(*group::ini_file());
}

void spline::set_bundle(bundle b)
{
    if (b)
        s = std::unique_ptr<settings>(new settings(b));
    else
        s = std::unique_ptr<settings>(new settings(options::make_bundle("")));

    last_input_value = QPointF(0, 0);
    activep = false;

    update_interp_data();
}

int spline::precision(const QList<QPointF>& points) const
{
    if (points.size())
        return int(value_count / clamp(points[points.size() - 1].x(), 1, max_x));

    return value_count / clamp(int(max_x), 1, value_count);
}
