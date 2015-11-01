/* Copyright (c) 2012-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QtGlobal>
#include <QList>
#include <QPointF>
#include <QString>
#include <QSettings>
#include <QMutex>
#include <vector>
#include <limits>
#include "opentrack-compat/qcopyable-mutex.hpp"

#ifdef BUILD_opentrack_spline_widget
#   define SPLINE_WIDGET_EXPORT Q_DECL_EXPORT
#else
#   define SPLINE_WIDGET_EXPORT Q_DECL_IMPORT
#endif

class SPLINE_WIDGET_EXPORT Map {
private:
    static constexpr int value_count = 10000;

    struct State {
        QList<QPointF> input;
        std::vector<float> data;        
    };

    int precision() const;
    void reload();
    float getValueInternal(int x);

    MyMutex _mutex;
    QPointF last_input_value;
    volatile bool activep;
    double max_x;
    double max_y;

    State cur, saved;
public:
    double maxInput() const { return max_x; }
    double maxOutput() const { return max_y; }
    Map();
    Map(double maxx, double maxy)
    {
        setMaxInput(maxx);
        setMaxOutput(maxy);
        reload();
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
    void setMaxInput(double MaxInput) {
        max_x = MaxInput;
    }
    void setMaxOutput(double MaxOutput) {
        max_y = MaxOutput;
    }

    void saveSettings(QSettings& settings, const QString& title);
    void loadSettings(QSettings& settings, const QString& title);
    void invalidate_unsaved_settings();

    void setTrackingActive(bool blnActive);
};
