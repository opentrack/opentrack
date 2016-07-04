/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "functionconfig.h"
#include <QMutexLocker>
#include <QCoreApplication>
#include <QPointF>
#include <QList>
#include <QtAlgorithms>
#include <QtAlgorithms>
#include <QSettings>
#include <QPixmap>
#include <QString>
#include <algorithm>
#include <cmath>

#include <QDebug>

void Map::setTrackingActive(bool blnActive)
{
    activep = blnActive;
}

Map::Map() : Map(0, 0)
{
}

void Map::removeAllPoints()
{
    QMutexLocker foo(&_mutex);
    cur.input.clear();
    reload();
}

void Map::setMaxInput(qreal max_input)
{
    QMutexLocker l(&_mutex);
    max_x = max_input;
}

void Map::setMaxOutput(qreal max_output)
{
    QMutexLocker l(&_mutex);
    max_y = max_output;
}

qreal Map::maxInput() const
{
    QMutexLocker l(&_mutex);
    return max_x;
}

qreal Map::maxOutput() const
{
    QMutexLocker l(&_mutex);
    return max_y;
}

Map::Map(qreal maxx, qreal maxy) :
    _mutex(QMutex::Recursive),
    max_x(0),
    max_y(0),
    activep(false)
{
    setMaxInput(maxx);
    setMaxOutput(maxy);
    if (cur.input.size() == 0)
        cur.input.push_back(QPointF(maxx, maxy));
    reload();
}

float Map::getValue(float x)
{
    QMutexLocker foo(&_mutex);
    float q  = x * precision();
    int    xi = (int)q;
    float  yi = getValueInternal(xi);
    float  yiplus1 = getValueInternal(xi+1);
    float  f = (q-xi);
    float  ret = yiplus1 * f + yi * (1.0f - f); // at least do a linear interpolation.
    last_input_value.setX(std::fabs(x));
    last_input_value.setY(std::fabs(ret));
    return ret;
}

bool Map::getLastPoint(QPointF& point )
{
    QMutexLocker foo(&_mutex);
    point = last_input_value;
    return activep;
}

float Map::getValueInternal(int x)
{
    float sign = x < 0 ? -1 : 1;
    x = abs(x);
    float ret;
    unsigned sz = cur.data.size();
    if (sz == 0)
        ret = 0;
    else
        ret = cur.data[std::min(unsigned(x), sz-1u)];
    return ret * sign;
}

static QPointF ensureInBounds(const QList<QPointF>& points, int i)
{
    int siz = points.size();
    if (siz == 0 || i < 0)
        return QPointF(0, 0);
    if (siz > i)
        return points[i];
    return points[siz - 1];
}

static bool sortFn(const QPointF& one, const QPointF& two)
{
    return one.x() < two.x();
}

void Map::reload()
{
    if (cur.input.size())
    {
        std::stable_sort(cur.input.begin(), cur.input.end(), sortFn);

        QList<QPointF> input = cur.input;
        auto& data = cur.data;

        data = std::vector<float>(value_count);
        const float mult = precision();
        const unsigned mult_ = unsigned(mult * 30);

        const unsigned sz = data.size();

        for (unsigned i = 0; i < sz; i++)
            data[i] = -1;

        if (input.size() == 1 && input[0].x() > 1e-2)
        {
            const float x = float(input[0].x());
            const float y = float(input[0].y());
            const unsigned max = unsigned(x * mult);
            for (unsigned k = 0; k < max; k++) {
                if (k < sz)
                    data[k] = y * k / max;
            }
        }
        else if (input[0].x() > 1e-2)
            input.prepend(QPointF(0, 0));

        for (int i = 0; i < int(sz); i++)
        {
            const QPointF p0 = ensureInBounds(input, i - 1);
            const QPointF p1 = ensureInBounds(input, i);
            const QPointF p2 = ensureInBounds(input, i + 1);
            const QPointF p3 = ensureInBounds(input, i + 2);

            const float p0_x = p0.x(), p1_x = p1.x(), p2_x = p2.x(), p3_x = p3.x();
            const float p0_y = p0.y(), p1_y = p1.y(), p2_y = p2.y(), p3_y = p3.y();

            // multiplier helps fill in all the x's needed
            const unsigned end = std::min(sz, unsigned(p2_x * mult_));
            const unsigned start = unsigned(p1_x * mult);

            for (unsigned j = start; j < end; j++)
            {
                const float t = (j - start) / (float) (end - start);
                const float t2 = t*t;
                const float t3 = t*t*t;

                const int x = .5f * ((2 * p1_x) +
                                     (-p0_x + p2_x) * t +
                                     (2 * p0_x - 5 * p1_x + 4 * p2_x - p3_x) * t2 +
                                     (-p0_x + 3 * p1_x - 3 * p2_x + p3_x) * t3)
                              * mult;

                const float y = .5f * ((2 * p1_y) +
                                       (-p0_y + p2_y) * t +
                                       (2 * p0_y - 5 * p1_y + 4 * p2_y - p3_y) * t2 +
                                       (-p0_y + 3 * p1_y - 3 * p2_y + p3_y) * t3);

                if (x >= 0 && x < (int)sz)
                    data[x] = y;
            }
        }

        float last = 0;
        for (unsigned i = 0; i < sz; i++)
        {
            if (data[i] < 0)
                data[i] = last;
            last = data[i];
        }
    }
    else
        cur.data.clear();
}

void Map::removePoint(int i)
{
    QMutexLocker foo(&_mutex);
    if (i >= 0 && i < cur.input.size())
    {
        cur.input.removeAt(i);
        reload();
    }
}

void Map::addPoint(QPointF pt)
{
    QMutexLocker foo(&_mutex);
    cur.input.append(pt);
    reload();
    std::stable_sort(cur.input.begin(), cur.input.end(), sortFn);
}

void Map::movePoint(int idx, QPointF pt)
{
    QMutexLocker foo(&_mutex);
    if (idx >= 0 && idx < cur.input.size())
    {
        cur.input[idx] = pt;
        // we don't allow points to be reordered, but sort due to possible caller logic error
        std::stable_sort(cur.input.begin(), cur.input.end(), sortFn);
        reload();
    }
}

const QList<QPointF> Map::getPoints()
{
    QMutexLocker foo(&_mutex);
    return cur.input;
}

void Map::invalidate_unsaved_settings()
{
    QMutexLocker foo(&_mutex);
    cur = saved;
    reload();
}

void Map::loadSettings(QSettings& settings, const QString& title)
{
    QMutexLocker foo(&_mutex);
    QPointF newPoint;
    QList<QPointF> points;
    settings.beginGroup(QString("Curves-%1").arg(title));

    int max = settings.value("point-count", 0).toInt();

    for (int i = 0; i < max; i++)
    {
        newPoint = QPointF(settings.value(QString("point-%1-x").arg(i), 0).toDouble(),
                           settings.value(QString("point-%1-y").arg(i), 0).toDouble());
        if (newPoint.x() > max_x)
            newPoint.setX(max_x);
        if (newPoint.y() > max_y)
            newPoint.setY(max_y);
        points.append(newPoint);
    }

    settings.endGroup();

    if (max == 0)
        points.append(QPointF(maxInput(), maxOutput()));

    cur.input = points;
    reload();
    saved = cur;
}

bool Map::State::operator==(const State& other) const
{
    if (input.size() != other.input.size())
        return false;

    const int sz = input.size();

    using std::fabs;

    for (int i = 0; i < sz; i++)
    {
        const qreal eps = 1e-3;

        if (fabs(input[i].x() - other.input[i].x()) > eps ||
            fabs(input[i].y() - other.input[i].y()) > eps)
        {
            return false;
        }
    }
    return true;
}

void Map::saveSettings(QSettings& settings, const QString& title)
{
    QMutexLocker foo(&_mutex);

    if (cur == saved)
        return;

    qDebug() << "spline-widget: saving" << title;

    settings.beginGroup(QStringLiteral("Curves-%1").arg(title));

    if (cur.input.size() == 0)
        cur.input.push_back(QPointF(max_x, max_y));

    const int max = cur.input.size();
    settings.setValue("point-count", max);

    for (int i = 0; i < max; i++)
    {
        settings.setValue(QString("point-%1-x").arg(i), cur.input[i].x());
        settings.setValue(QString("point-%1-y").arg(i), cur.input[i].y());
    }

    for (int i = max; true; i++)
    {
        QString x = QString("point-%1-x").arg(i);
        if (!settings.contains(x))
            break;
        settings.remove(x);
        settings.remove(QString("point-%1-y").arg(i));
    }

    saved = cur;

    settings.endGroup();
}


int Map::precision() const
{
    if (cur.input.size())
        return (value_count-1) / std::max<float>(1.f, (cur.input[cur.input.size() - 1].x()));
    return 1;
}
