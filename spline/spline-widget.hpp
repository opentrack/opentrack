/* Copyright (c) 2012-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

// Adapted to FaceTrackNoIR by Wim Vriend.

#pragma once

#include "spline.hpp"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

#include "export.hpp"

#include <QWidget>
#include <QRect>
#include <QPoint>
#include <QPointF>
#include <QToolTip>
#include <QShowEvent>
#include <QFocusEvent>
#include <QMetaObject>

#include <QDebug>

class OTR_SPLINE_EXPORT spline_widget final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier)
    Q_PROPERTY(bool is_preview_only READ is_preview_only WRITE set_preview_only)
    Q_PROPERTY(int x_step READ x_step WRITE set_x_step)
    Q_PROPERTY(int y_step READ y_step WRITE set_y_step)

    using points_t = spline::points_t;
public:
    spline_widget(QWidget *parent = 0);
    ~spline_widget();

    void setConfig(base_spline* spl);

    QColor colorBezier() const;
    void setColorBezier(QColor color);

    void force_redraw();
    void set_preview_only(bool val);
    bool is_preview_only() const;

    double x_step() { return _x_step; }
    double y_step() { return _y_step; }
    void set_x_step(double val) { _x_step = std::fmax(1., val); }
    void set_y_step(double val) { _y_step = std::fmax(1., val); }

    void set_snap(double x, double y) { snap_x = x; snap_y = y; }
    void get_snap(double& x, double& y) const { x = snap_x; y = snap_y; }
public slots:
    void reload_spline();
protected slots:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
private:
    int get_closeness_limit();
    void show_tooltip(const QPoint& pos, const QPointF& value = QPointF(0, 0));
    bool is_in_bounds(const QPoint& pos) const;

    void drawBackground();
    void drawFunction();
    void drawPoint(QPainter& painter, const QPointF& pt, const QColor& colBG, const QColor& border = QColor(50, 100, 120, 200));
    void drawLine(QPainter& painter, const QPoint& start, const QPoint& end, const QPen& pen);
    bool point_within_pixel(const QPointF& pt, const QPoint& pixel);

    void focusOutEvent(QFocusEvent*e) override;
    void resizeEvent(QResizeEvent *) override;

    bool is_on_pt(const QPoint& pos, int* pt = nullptr);
    void update_range();
    QPointF pixel_coord_to_point(const QPoint& point);

    QPointF point_to_pixel_(const QPointF& point);
    QPoint point_to_pixel(const QPointF& point);

    QPointF c;
    base_spline* _config;

    QPixmap _background;
    QPixmap _function;
    QColor spline_color;
    QColor widget_bg_color = palette().background().color();

    // bounds of the rectangle user can interact with
    QRect pixel_bounds;

    QMetaObject::Connection connection;

    double snap_x, snap_y;
    double _x_step, _y_step;
    int moving_control_point_idx;
    bool _draw_function, _preview_only;

    static constexpr int point_size = 4;
};
