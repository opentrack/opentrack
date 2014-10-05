/* Copyright (c) 2012, 2013 Stanisław Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include <QMutexLocker>
#include <QCoreApplication>
#include <QPointF>
#include <QList>
#include "functionconfig.h"
#include <QtAlgorithms>
#include <QtAlgorithms>
#include <QSettings>
#include <cmath>
#include <QPixmap>

void Map::setTrackingActive(bool blnActive)
{
    activep = blnActive;
}

Map::Map() :
    _mutex(QMutex::Recursive),
    data(0),
    activep(false),
    max_x(0),
    max_y(0)
{
}

float Map::getValue(float x) {
    QMutexLocker foo(&_mutex);
    int x2 = (int) (std::min<float>(std::max<float>(x, -360), 360) * MEMOIZE_PRECISION);
    float ret = getValueInternal(x2);
        last_input_value.setX(x);
        last_input_value.setY(ret);
        return ret;
}

bool Map::getLastPoint(QPointF& point ) {
    QMutexLocker foo(&_mutex);
        point = last_input_value;
        return activep;
}

float Map::getValueInternal(int x) {
    float sign = x < 0 ? -1 : 1;
    x = x < 0 ? -x : x;
    float ret;
    int sz = data.size();
        if (sz == 0)
                ret = 0;
        else if (x < 0)
                ret = 0;
        else if (x < sz && x >= 0)
                ret = data[x];
        else
                ret = data[sz - 1];
        return ret * sign;
}

static __inline QPointF ensureInBounds(QList<QPointF> points, int i) {
        int siz = points.size();
        if (siz == 0 || i < 0)
                return QPointF(0, 0);
        if (siz > i)
                return points[i];
        return points[siz - 1];
}

static bool sortFn(const QPointF& one, const QPointF& two) {
        return one.x() < two.x();
}

void Map::reload() {
        if (input.size())
                qStableSort(input.begin(), input.end(), sortFn);

        if (input.size()) {
        data = std::vector<float>(MEMOIZE_PRECISION * input[input.size() - 1].x());
        
        const int sz = data.size();

        for (int i = 0; i < sz; i++)
                data[i] = -1;

                for (int k = 0; k < input[0].x() * MEMOIZE_PRECISION; k++) {
            if (k < sz)
                data[k] = input[0].y() * k / (input[0].x() * MEMOIZE_PRECISION);
        }

       for (int i = 0; i < sz; i++) {
            QPointF p0 = ensureInBounds(input, i - 1);
            QPointF p1 = ensureInBounds(input, i);
            QPointF p2 = ensureInBounds(input, i + 1);
            QPointF p3 = ensureInBounds(input, i + 2);

            int end = std::min<int>(sz, p2.x() * MEMOIZE_PRECISION);
            int start = p1.x() * MEMOIZE_PRECISION;

            for (int j = start; j < end; j++) {
                double t = (j - start) / (double) (end - start);
                double t2 = t*t;
                double t3 = t*t*t;

                int x = .5 * ((2. * p1.x()) +
                              (-p0.x() + p2.x()) * t +
                              (2. * p0.x() - 5. * p1.x() + 4. * p2.x() - p3.x()) * t2 +
                              (-p0.x() + 3. * p1.x() - 3. * p2.x() + p3.x()) * t3)
                        * MEMOIZE_PRECISION;

                float y = .5 * ((2. * p1.y()) +
                                 (-p0.y() + p2.y()) * t +
                                 (2. * p0.y() - 5. * p1.y() + 4. * p2.y() - p3.y()) * t2 +
                                 (-p0.y() + 3. * p1.y() - 3. * p2.y() + p3.y()) * t3);

                if (x >= 0 && x < sz)
                    data[x] = y;
            }
                }

       float last = 0;
       for (int i = 0; i < sz; i++)
       {
           if (data[i] <= 0)
               data[i] = last;
           last = data[i];
       }
        }
}

void Map::removePoint(int i) {
    QMutexLocker foo(&_mutex);
    if (i >= 0 && i < input.size())
    {
        input.removeAt(i);
        reload();
    }
}

void Map::addPoint(QPointF pt) {
    QMutexLocker foo(&_mutex);
        input.append(pt);
        reload();
}

void Map::movePoint(int idx, QPointF pt) {
    QMutexLocker foo(&_mutex);
    if (idx >= 0 && idx < input.size())
    {
        input[idx] = pt;
        reload();
    }
}

const QList<QPointF> Map::getPoints() {
    QMutexLocker foo(&_mutex);
    // NB can't pass by reference
        return input;
}

void Map::loadSettings(QSettings& settings, const QString& title) {
    QMutexLocker foo(&_mutex);
    QPointF newPoint;

        QList<QPointF> points;
        settings.beginGroup(QString("Curves-%1").arg(title));
        
    int max = settings.value("point-count", 0).toInt();

        for (int i = 0; i < max; i++) {
                newPoint = QPointF(settings.value(QString("point-%1-x").arg(i), 0).toFloat(),
                                   settings.value(QString("point-%1-y").arg(i), 0).toFloat());
                if (newPoint.x() > max_x) {
                        newPoint.setX(max_x);
                }
                if (newPoint.y() > max_y) {
                        newPoint.setY(max_y);
                }
                points.append(newPoint);
        }
    settings.endGroup();
        input = points;
        reload();
}

void Map::saveSettings(QSettings& settings, const QString& title) {
    QMutexLocker foo(&_mutex);
        settings.beginGroup(QString("Curves-%1").arg(title));
        int max = input.size();
        settings.setValue("point-count", max);
        for (int i = 0; i < max; i++) {
                settings.setValue(QString("point-%1-x").arg(i), input[i].x());
                settings.setValue(QString("point-%1-y").arg(i), input[i].y());
    }

    for (int i = max; true; i++)
    {
        QString x = QString("point-%1-x").arg(i);
        if (!settings.contains(x))
            break;
        settings.remove(x);
        settings.remove(QString("point-%1-y").arg(i));
    }
        settings.endGroup();
}
