#include <QMutexLocker>
#include <QCoreApplication>
#include <QPointF>
#include <QList>
#include "functionconfig.h"
#include <QtAlgorithms>
#include <QtAlgorithms>
#include <QSettings>
#include <QPixmap>
#include <algorithm>
#include <cmath>

void Map::setTrackingActive(bool blnActive)
{
    activep = blnActive;
}

Map::Map() :
    _mutex(QMutex::Recursive),
    activep(false),
    max_x(0),
    max_y(0)
{
}

float Map::getValue(float x) {
    QMutexLocker foo(&_mutex);
    float q  = x * precision();
    int    xi = (int)q;
    float  yi = getValueInternal(xi);
    float  yiplus1 = getValueInternal(xi+1);
    float  f = (q-xi);
    float  ret = yiplus1 * f + yi * (1.0f - f); // at least do a linear interpolation.
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
    x = abs(x);
    float ret;
    int sz = cur.data.size();
    if (sz == 0)
        ret = 0;
    else
        ret = cur.data[std::min<unsigned>(x, sz-1)];
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
    if (cur.input.size())
    {
        qStableSort(cur.input.begin(), cur.input.end(), sortFn);

        QList<QPointF> input = cur.input;
        auto& data = cur.data;

        data = std::vector<float>(value_count);
        const int mult = precision();
        
        const int sz = data.size();
        
        for (int i = 0; i < sz; i++)
            data[i] = -1;
        
        if (input.size() == 1)
        {
            for (int k = 0; k < input[0].x() * mult; k++) {
                if (k < sz)
                    data[k] = input[0].y() * k / (input[0].x() * mult);
            }
        }
        else if (input[0].x() > 1e-2)
            input.prepend(QPointF(0, 0));
        
        for (int i = 0; i < sz; i++) {
            QPointF p0 = ensureInBounds(input, i - 1);
            QPointF p1 = ensureInBounds(input, i);
            QPointF p2 = ensureInBounds(input, i + 1);
            QPointF p3 = ensureInBounds(input, i + 2);

            const float p0_x = p0.x(), p1_x = p1.x(), p2_x = p2.x(), p3_x = p3.x();
            const float p0_y = p0.y(), p1_y = p1.y(), p2_y = p2.y(), p3_y = p3.y();
            
            int end = std::min<int>(sz, p2.x() * mult);
            int start = p1.x() * mult;
            
            for (int j = start; j < end; j++) {
                float t = (j - start) / (float) (end - start);
                float t2 = t*t;
                float t3 = t*t*t;

                int x = .5 * ((2. * p1_x) +
                              (-p0_x + p2_x) * t +
                              (2. * p0_x - 5. * p1_x + 4. * p2_x - p3_x) * t2 +
                              (-p0_x + 3. * p1_x - 3. * p2_x + p3_x) * t3)
                        * mult;
                
                float y = .5 * ((2. * p1_y) +
                                (-p0_y + p2_y) * t +
                                (2. * p0_y - 5. * p1_y + 4. * p2_y - p3_y) * t2 +
                                (-p0_y + 3. * p1_y - 3. * p2_y + p3_y) * t3);
                
                if (x >= 0 && x < sz)
                    data[x] = y;
            }
        }
        
        float last = 0;
        for (int i = 0; i < sz; i++)
        {
            if (data[i] < 0)
                data[i] = last;
            last = data[i];
        }
    }
    else
        cur.data.clear();
}

void Map::removePoint(int i) {
    QMutexLocker foo(&_mutex);
    if (i >= 0 && i < cur.input.size())
    {
        cur.input.removeAt(i);
        reload();
    }
}

void Map::addPoint(QPointF pt) {
    QMutexLocker foo(&_mutex);
        cur.input.append(pt);
        reload();
}

void Map::movePoint(int idx, QPointF pt) {
    QMutexLocker foo(&_mutex);
    if (idx >= 0 && idx < cur.input.size())
    {
        cur.input[idx] = pt;
        reload();
    }
}

const QList<QPointF> Map::getPoints() {
    QMutexLocker foo(&_mutex);
    return cur.input;
}

void Map::invalidate_unsaved_settings()
{
    cur = saved;
    reload();
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
    
    if (max == 0)
        points.append(QPointF(maxInput(), maxOutput()));
    
    cur.input = points;
    reload();
    saved = cur;
}

void Map::saveSettings(QSettings& settings, const QString& title) {
    QMutexLocker foo(&_mutex);
    settings.beginGroup(QString("Curves-%1").arg(title));
    int max = cur.input.size();
    settings.setValue("point-count", max);

    for (int i = 0; i < max; i++) {
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


int Map::precision() const {
    if (cur.input.size())
        return value_count / std::max<float>(1.f, (cur.input[cur.input.size() - 1].x()));
    return 1;
}
