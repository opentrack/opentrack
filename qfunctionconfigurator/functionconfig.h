/* Copyright (c) 2011-2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QList>
#include <QPointF>
#include <QString>
#include <QSettings>
#include <QMutex>
#include <vector>
#include "../facetracknoir/plugin-api.hpp"
#include "../facetracknoir/qcopyable-mutex.hpp"


class OPENTRACK_EXPORT Map {
private:
    struct State {
        QList<QPointF> input;
        std::vector<float> data;        
    };
    
    static constexpr long MEMOIZE_PRECISION = 25;
    void reload();
    float getValueInternal(int x);

    MyMutex _mutex;
    QPointF last_input_value;
    volatile bool activep;
    int max_x;
    int max_y;
    
    State cur, saved;
public:
    int maxInput() const { return max_x; }
    int maxOutput() const { return max_y; }
    Map();
    Map(int maxx, int maxy)
    {
        setMaxInput(maxx);
        setMaxOutput(maxy);
    }

    float getValue(float x);
    bool getLastPoint(QPointF& point);
    void removePoint(int i);
    void removeAllPoints() {
        QMutexLocker foo(&_mutex);
        cur.input.clear();
        reload();
    }

    void addPoint(QPointF pt);
    void movePoint(int idx, QPointF pt);
    const QList<QPointF> getPoints();
    void setMaxInput(int MaxInput) {
        max_x = MaxInput;
    }
    void setMaxOutput(int MaxOutput) {
        max_y = MaxOutput;
    }

    void saveSettings(QSettings& settings, const QString& title);
    void loadSettings(QSettings& settings, const QString& title);
    void invalidate_unsaved_settings();

    void setTrackingActive(bool blnActive);
};
