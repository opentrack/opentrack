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
#include <QtGui>
#include <QMetaObject>
#include <QPointF>
#include <QDebug>

class OPENTRACK_SPLINE_EXPORT spline_widget final : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier)
    Q_PROPERTY(bool is_preview_only READ is_preview_only WRITE set_preview_only)

    using points_t = spline::points_t;
public:
    spline_widget(QWidget *parent = 0);

    spline* config();

    void setConfig(spline* spl);

    QColor colorBezier() const
    {
        return spline_color;
    }
    void setColorBezier(QColor color)
    {
        spline_color = color;
        update();
    }
    void force_redraw()
    {
        _background = QPixmap();
        update();
    }
    void set_preview_only(bool val);
    bool is_preview_only() const;
    void set_snap(int x, int y) { snap_x = x; snap_y = y; }
    void get_snap(int& x, int& y) const { x = snap_x; y = snap_y; }
protected slots:
    void paintEvent(QPaintEvent *e) override;
    void mousePressEvent(QMouseEvent *e) override;
    void mouseMoveEvent(QMouseEvent *e) override;
    void mouseReleaseEvent(QMouseEvent *e) override;
    void reload_spline();
private:
    void drawBackground();
    void drawFunction();
    void drawPoint(QPainter *painter, const QPointF &pt, QColor colBG, QColor border = QColor(50, 100, 120, 200));
    void drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen& pen);
    bool point_within_pixel(const QPointF& pt, const QPointF& pixel);
protected:
    void resizeEvent(QResizeEvent *) override;
private:
    void update_range();

    QPointF pixel_coord_to_point (const QPointF& point);
    QPointF point_to_pixel(const QPointF& point);

    spline* _config;

    // bounds of the rectangle user can interact with
    QRectF  pixel_bounds;

    QPointF c;

    QColor spline_color;

    QPixmap _background;
    QPixmap _function;

    QMetaObject::Connection connection;

    int moving_control_point_idx;

    int snap_x, snap_y;
    bool _draw_function, _preview_only;

    static constexpr int line_length_pixels = 3;
    static constexpr int point_size = 4;
    static constexpr int point_closeness_limit = 5;
};
