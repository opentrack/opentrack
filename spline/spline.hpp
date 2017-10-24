/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "compat/copyable-mutex.hpp"
#include "options/options.hpp"
#include "compat/util.hpp"
using namespace options;

#include "axis-opts.hpp"

#include "export.hpp"

#include <vector>
#include <limits>
#include <memory>
#include <functional>

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
    axis_opts opts;
    settings(bundle b, const QString& axis_name, Axis idx);
    ~settings() override;
signals:
    void recomputed() const;
};

} // ns spline_detail

class OTR_SPLINE_EXPORT spline final
{
    double bucket_size_coefficient(const QList<QPointF>& points) const;
    void update_interp_data();
    float get_value_internal(int x);
    void add_lone_point();
    float get_value_no_save_internal(double x);
    static bool sort_fn(const QPointF& one, const QPointF& two);

    static QPointF ensure_in_bounds(const QList<QPointF>& points, int i);
    static int element_count(const QList<QPointF>& points, double max_input);

    std::shared_ptr<spline_detail::settings> s;
    QMetaObject::Connection connection, conn_maxx, conn_maxy;

    static constexpr std::size_t value_count = 4096;

    std::vector<float> data = std::vector<float>(value_count, float(-16));

    mutex _mutex { mutex::recursive };
    QPointF last_input_value;
    std::shared_ptr<QObject> ctx { std::make_shared<QObject>() };

    Axis axis = NonAxis;

    bool activep = false;
    bool validp = false;

public:
    void invalidate_settings();

    void reload();
    void save();
    void set_bundle(bundle b, const QString& axis_name, Axis axis);

    double max_input() const;
    double max_output() const;

    spline();
    spline(const QString& name, const QString& axis_name, Axis axis);
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

    void set_tracking_active(bool value);
    bundle get_bundle();
    void ensure_valid(QList<QPointF>& the_points);

    std::shared_ptr<spline_detail::settings> get_settings();
    std::shared_ptr<const spline_detail::settings> get_settings() const;

    using points_t = decltype(s->points());
    int get_point_count() const;

    using settings = spline_detail::settings;
};
