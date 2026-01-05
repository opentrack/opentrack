/* Copyright (c) 2012-2019, Stanislaw Halik <sthalik@misaki.pl>

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

#include <QObject>
#include <QPointF>
#include <QString>
#include <QMetaObject>

namespace spline_detail {

using points_t = QList<QPointF>;
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
    base_spline_() = default;
    virtual ~base_spline_();

    virtual double get_value(double x) const = 0;
    virtual double get_value_no_save(double x) const = 0;

    [[nodiscard]] virtual bool get_last_value(QPointF& point) = 0;
    virtual void set_tracking_active(bool value) const = 0;

    virtual double max_input() const = 0;
    virtual double max_output() const = 0;

    virtual const points_t& get_points() const = 0;
    virtual int get_point_count() const = 0;

    base_spline_(const base_spline_&) = default;
    base_spline_& operator=(const base_spline_&) = default;
};

struct OTR_SPLINE_EXPORT spline_settings_mixin
{
    virtual std::shared_ptr<base_settings> get_settings() = 0;
    virtual std::shared_ptr<const base_settings> get_settings() const = 0;

    spline_settings_mixin(const spline_settings_mixin&) = default;
    spline_settings_mixin& operator=(const spline_settings_mixin&) = default;

    spline_settings_mixin() = default;
    virtual ~spline_settings_mixin();
};

struct OTR_SPLINE_EXPORT spline_modify_mixin
{
    virtual void add_point(QPointF pt) = 0;
    virtual void add_point(double x, double y) = 0;
    virtual void move_point(int idx, QPointF pt) = 0;
    virtual void remove_point(int i) = 0;
    virtual void clear() = 0;

    spline_modify_mixin(const spline_modify_mixin&) = default;
    spline_modify_mixin& operator=(const spline_modify_mixin&) = default;

    spline_modify_mixin() = default;
    virtual ~spline_modify_mixin();
};

struct OTR_SPLINE_EXPORT base_spline : base_spline_, spline_modify_mixin, spline_settings_mixin
{
    base_spline(const base_spline&) = default;
    base_spline& operator=(const base_spline&) = default;

    base_spline() = default;
    ~base_spline() override;
};

class OTR_SPLINE_EXPORT spline : public base_spline
{
    using f = double;

    double bucket_size_coefficient(const QList<QPointF>& points) const;
    void update_interp_data() const;
    double get_value_internal(int x) const;
    static bool sort_fn(QPointF one, QPointF two);

    static void ensure_in_bounds(const QList<QPointF>& points, int i, f& x, f& y);
    static int element_count(const QList<QPointF>& points, double max_input);

    void disconnect_signals();
    void invalidate_settings_();

    mutex<QRecursiveMutex> mtx;
    std::shared_ptr<settings> s;
    QMetaObject::Connection conn_points, conn_maxx, conn_maxy;

    std::shared_ptr<QObject> ctx { std::make_shared<QObject>() };

    mutable QPointF last_input_value{-1, -1};
    mutable std::vector<f> data = std::vector<f>(value_count, magic_fill_value);
    mutable points_t points;
    mutable axis_opts::max_clamp clamp_x = axis_opts::x1000, clamp_y = axis_opts::x1000;
    mutable bool activep = false;

    static constexpr unsigned value_count = 16384;
    static constexpr f magic_fill_value = -(1 << 24) + 1;
    static constexpr double c_interp = 5;

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

    spline(const spline&) = default;

    double get_value(double x) const override;
    double get_value_no_save(double x) const override;
    [[nodiscard]] bool get_last_value(QPointF& point) override;

    void add_point(QPointF pt) override;
    void add_point(double x, double y) override;
    void move_point(int idx, QPointF pt) override;
    void remove_point(int i) override;
    void clear() override;

    const points_t& get_points() const override;

    void set_tracking_active(bool value) const override;
    bundle get_bundle();
    void ensure_valid(points_t& in_out) const;

    std::shared_ptr<base_settings> get_settings() override;
    std::shared_ptr<const base_settings> get_settings() const override;

    int get_point_count() const override;
};

} // ns spline_detail

using spline = spline_detail::spline;
