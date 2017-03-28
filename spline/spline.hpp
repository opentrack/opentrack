/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "compat/qcopyable-mutex.hpp"
#include "options/options.hpp"
#include "compat/util.hpp"
using namespace options;

#include "export.hpp"

#include <vector>
#include <limits>
#include <memory>

#include <QObject>
#include <QPointF>
#include <QString>
#include <QMetaObject>

namespace spline_detail {

class OTR_SPLINE_EXPORT settings final : public QObject
{
    Q_OBJECT
public:
    bundle b;
    value<QList<QPointF>> points;
    settings(bundle b);
    ~settings() override;
signals:
    void recomputed() const;
};

}

class OTR_SPLINE_EXPORT spline final
{
    double precision(const QList<QPointF>& points) const;
    void update_interp_data();
    float get_value_internal(int x);
    void add_lone_point();
    float get_value_no_save_internal(double x);
    static bool sort_fn(const QPointF& one, const QPointF& two);

    static QPointF ensure_in_bounds(const QList<QPointF>& points, double max_x, int i);
    static int element_count(const QList<QPointF>& points, double max_x);

    mem<spline_detail::settings> s;
    QMetaObject::Connection connection;

    std::vector<float> data;
    using interp_data_t = decltype(data);

    static constexpr int value_count = 4096;

    MyMutex _mutex;
    QPointF last_input_value;
    qreal max_x, max_y;
    volatile bool activep;
    bool validp;

public:
    using settings = spline_detail::settings;

    void reload();
    void save(QSettings& s);
    void save();
    void set_bundle(bundle b);

    qreal max_input() const;
    qreal max_output() const;
    spline();
    spline(qreal maxx, qreal maxy, const QString& name);
    ~spline();

    spline& operator=(const spline&) = default;
    spline(const spline&) = default;

    float get_value(double x);
    float get_value_no_save(double x) const;
    DEFUN_WARN_UNUSED bool get_last_value(QPointF& point);
    void remove_point(int i);
    void clear();

    void add_point(QPointF pt);
    void add_point(double x, double y);
    void move_point(int idx, QPointF pt);
    QList<QPointF> get_points() const;
    void set_max_input(qreal MaxInput);
    void set_max_output(qreal MaxOutput);

    void set_tracking_active(bool value);
    bundle get_bundle();
    void recompute();

    mem<settings> get_settings();
    mem<const settings> get_settings() const;

    using points_t = decltype(s->points());
    int get_point_count() const;
};
