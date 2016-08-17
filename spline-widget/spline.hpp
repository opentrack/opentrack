/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QObject>
#include <QPointF>
#include <QString>
#include <QMutex>
#include <vector>
#include <limits>
#include <memory>
#include "compat/qcopyable-mutex.hpp"
#include "options/options.hpp"
using namespace options;

#ifdef BUILD_spline_widget
#   define SPLINE_WIDGET_EXPORT Q_DECL_EXPORT
#else
#   define SPLINE_WIDGET_EXPORT Q_DECL_IMPORT
#endif

class SPLINE_WIDGET_EXPORT spline final
{
private:
    struct settings
    {
        bundle b;
        value<QList<QPointF>> points;
        settings(bundle b) :
            b(b),
            points(b, "points", QList<QPointF>())
        {}
    };

    ptr<settings> s;

    std::vector<float> data;
    using interp_data_t = decltype(data);

    static constexpr int value_count = 10000;

    int precision(const QList<QPointF>& points) const;
    void update_interp_data();
    float getValueInternal(int x);
    void add_lone_point();
    static bool sort_fn(const QPointF& one, const QPointF& two);

    static QPointF ensure_in_bounds(const QList<QPointF>& points, int i);

    MyMutex _mutex;
    QPointF last_input_value;
    qreal max_x, max_y;
    volatile bool activep;

    template<typename t, typename u, typename w>
    static inline auto clamp(t val, u min, w max) -> decltype (val * min * max)
    {
        if (val > max)
            return max;
        if (val < min)
            return min;
        return val;
    }

public:
    void reload();
    void save(QSettings& s);
    void save();
    void set_bundle(bundle b);

    qreal maxInput() const;
    qreal maxOutput() const;
    spline();
    spline(qreal maxx, qreal maxy, const QString& name);

    float getValue(double x);
    bool getLastPoint(QPointF& point);
    void removePoint(int i);
    void removeAllPoints();

    void addPoint(QPointF pt);
    void movePoint(int idx, QPointF pt);
    QList<QPointF> getPoints() const;
    void setMaxInput(qreal MaxInput);
    void setMaxOutput(qreal MaxOutput);

    void setTrackingActive(bool blnActive);
    bundle get_bundle() { return s->b; }

    using points_t = decltype(s->points.get());
};
