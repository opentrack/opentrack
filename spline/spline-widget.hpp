/* Copyright (c) 2012-2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

// Adapted to FaceTrackNoIR by Wim Vriend.

#pragma once

#include "spline.hpp"
#include "api/plugin-api.hpp"
#include "compat/qt-dpi.hpp"
#include "options/options.hpp"

#include "export.hpp"

#include <cmath>

#include <QWidget>
#include <QMetaObject>

#include <QDebug>

namespace spline_detail {

using namespace options;

class OTR_SPLINE_EXPORT spline_widget final : public QWidget, public screen_dpi_mixin<spline_widget>
{
    Q_OBJECT
    Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier)
    Q_PROPERTY(bool is_preview_only READ is_preview_only WRITE set_preview_only)
    Q_PROPERTY(int x_step READ x_step WRITE set_x_step)
    Q_PROPERTY(int y_step READ y_step WRITE set_y_step)

public:
    explicit spline_widget(QWidget *parent = nullptr);
    ~spline_widget() override;

    void set_config(base_spline* spl);

    QColor colorBezier() const;
    void setColorBezier(QColor const& color);

    void force_redraw();
    void set_preview_only(bool val);
    bool is_preview_only() const;

    double x_step() const { return x_step_; }
    double y_step() const { return y_step_; }
    void set_x_step(double val) { x_step_ = std::fmax(1., val); }
    void set_y_step(double val) { y_step_ = std::fmax(1., val); }

    void set_snap(double x, double y) { snap_x = x; snap_y = y; }
    void get_snap(double& x, double& y) const { x = snap_x; y = snap_y; }

    QSize minimumSizeHint() const override;
public slots:
    void reload_spline();
protected slots:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    double min_pt_distance() const;
    void show_tooltip(const QPoint& pos, const QPointF& value = QPointF(0, 0));
    bool is_in_bounds(const QPointF& pos) const;

    void drawBackground();
    void drawFunction();
    void drawPoint(QPainter& painter, const QPointF& pt, const QColor& colBG, const QColor& border = QColor(50, 100, 120, 200));
    void drawLine(QPainter& painter, const QPointF& start, const QPointF& end, const QPen& pen);
    bool point_within_pixel(const QPointF& pt, const QPointF& pixel);

    void focusOutEvent(QFocusEvent*e) override;
    void resizeEvent(QResizeEvent *) override;

    bool is_on_pt(const QPointF& pos, int* pt = nullptr);
    void update_range();
    void changeEvent(QEvent* e) override;

    QPointF pixel_to_point(const QPointF& point);
    QPointF point_to_pixel(const QPointF& point);

    static double snap(double x, double snap_value);

    QPointF c;
    base_spline* config = nullptr;

    QPixmap background_img;
    QPixmap spline_img;
    QColor spline_color;
    QColor widget_bg_color = palette().window().color();

    // bounds of the rectangle user can interact with
    QRect pixel_bounds;

    QMetaObject::Connection connection;

    double snap_x = 0, snap_y = 0;
    double x_step_ = 10, y_step_ = 10;
    int moving_control_point_idx = -1;
    bool draw_function = true, preview_only = false;

    // point's circle radius on the widget
    static constexpr int point_size_in_pixels_ = 5;

    const double point_size_in_pixels = point_size_in_pixels_ * screen_dpi();
};

} // ns spline_detail

using spline_widget = spline_detail::spline_widget;
