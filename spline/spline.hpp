/* Copyright (c) 2012-2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
#include "axis-opts.hpp"
#include "export.hpp"
#include "compat/mutex.hpp"

#include <cstddef>
#include <vector>
#include <limits>
#include <memory>
#include <functional>

#include <QObject>
#include <QPointF>
#include <QString>
#include <QMetaObject>

namespace spline_detail {

using namespace options;

class OTR_SPLINE_EXPORT base_settings : public QObject
{
    Q_OBJECT

signals:
    void recomputed() const;
};

class OTR_SPLINE_EXPORT settings final : public base_settings
{
public:
    bundle b;
    value<QList<QPointF>> points { b, "points", {} };
    axis_opts opts;
    settings(bundle const& b, const QString& axis_name, Axis idx);
    ~settings() override;
};

struct OTR_SPLINE_EXPORT base_spline_
{
    virtual ~base_spline_();
    base_spline_& operator=(const base_spline_&) = default;

    virtual float get_value(double x) = 0;
    virtual float get_value_no_save(double x) const = 0;

    [[nodiscard]] virtual bool get_last_value(QPointF& point) = 0;
    virtual void set_tracking_active(bool value) = 0;

    virtual double max_input() const = 0;
    virtual double max_output() const = 0;

    using points_t = QList<QPointF>;

    virtual points_t const& get_points() const = 0;
    virtual int get_point_count() const = 0;
};

struct OTR_SPLINE_EXPORT spline_settings_mixin
{
    using base_settings = spline_detail::base_settings;

    virtual std::shared_ptr<spline_detail::base_settings> get_settings() = 0;
    virtual std::shared_ptr<const spline_detail::base_settings> get_settings() const = 0;

    virtual ~spline_settings_mixin();
};

struct OTR_SPLINE_EXPORT spline_modify_mixin
{
    virtual void add_point(QPointF pt) = 0;
    virtual void add_point(double x, double y) = 0;
    virtual void move_point(int idx, QPointF pt) = 0;
    virtual void remove_point(int i) = 0;
    virtual void clear() = 0;

    virtual ~spline_modify_mixin();
};

struct OTR_SPLINE_EXPORT base_spline : base_spline_, spline_modify_mixin, spline_settings_mixin
{
    ~base_spline() override;
};

class OTR_SPLINE_EXPORT spline : public base_spline
{
    double bucket_size_coefficient(const QList<QPointF>& points) const;
    void update_interp_data() const;
    float get_value_internal(int x) const;
    static bool sort_fn(const QPointF& one, const QPointF& two);

    static QPointF ensure_in_bounds(const QList<QPointF>& points, int i);
    static int element_count(const QList<QPointF>& points, double max_input);

    void disconnect_signals();

    mutex mtx { mutex::Recursive };
    std::shared_ptr<spline_detail::settings> s;
    QMetaObject::Connection conn_changed, conn_maxx, conn_maxy;
    mutable std::vector<float> data = std::vector<float>(value_count, float(-16));
    mutable QPointF last_input_value;

    std::shared_ptr<QObject> ctx { std::make_shared<QObject>() };

    // cached s->points
    mutable points_t points;

    static constexpr inline std::size_t value_count = 4096;

    mutable bool activep = false;
    mutable bool validp = false;

public:
    void invalidate_settings();

    void reload();
    void save();
    void set_bundle(bundle b, const QString& axis_name, Axis axis);

    double max_input() const override;
    double max_output() const override;

    spline();
    spline(const QString& name, const QString& axis_name, Axis axis);
    ~spline() override;

    spline& operator=(const spline&) = default;
    spline(const spline&) = default;

    float get_value(double x) override;
    float get_value_no_save(double x) const override;
    [[nodiscard]] bool get_last_value(QPointF& point) override;

    void add_point(QPointF pt) override;
    void add_point(double x, double y) override;
    void move_point(int idx, QPointF pt) override;
    void remove_point(int i) override;
    void clear() override;

    points_t const& get_points() const override;

    void set_tracking_active(bool value) override;
    bundle get_bundle();
    void ensure_valid(points_t& in_out) const;

    std::shared_ptr<spline_detail::base_settings> get_settings() override;
    std::shared_ptr<const spline_detail::base_settings> get_settings() const override;

    int get_point_count() const override;

    using settings = spline_detail::settings;
};

} // ns spline_detail

using spline = spline_detail::spline;
