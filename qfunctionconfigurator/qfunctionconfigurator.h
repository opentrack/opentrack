/* Copyright (c) 2011-2014 Stanislaw Halik <sthalik@misaki.pl>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

// Adapted to FaceTrackNoIR by Wim Vriend.

#pragma once

#include <QWidget>
#include <QtGui>
#include <QPointF>
#include <QElapsedTimer>
#include "qfunctionconfigurator/functionconfig.h"
#include "facetracknoir/plugin-api.hpp"

class OPENTRACK_EXPORT QFunctionConfigurator : public QWidget
{
	Q_OBJECT
    Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier)
public:
	QFunctionConfigurator(QWidget *parent = 0);
    
	Map* config();
    void setConfig(Map* config, const QString &name);
    
    QColor colorBezier() const
    {
        return spline_color;
    }
    void setColorBezier(QColor color)
    {
        spline_color = color;
        update();
    }
protected slots:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
private:
    void drawBackground();
	void drawFunction();
	void drawPoint(QPainter *painter, const QPointF &pt, QColor colBG );
	void drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen& pen);
    bool point_within_pixel(const QPointF& pt, const QPointF& pixel);
protected:
	void resizeEvent(QResizeEvent *) override;
private:
    void update_range();

    QPointF pixel_coord_to_point (const QPointF& point);
    QPointF point_to_pixel (const QPointF& point);

    Map* _config;
    
    // bounds of the rectangle user can interact with
	QRectF  pixel_bounds;
    
    int moving_control_point_idx;
    QElapsedTimer timer;
    QPointF c;

	QColor spline_color;
    
	QPixmap _background;
	QPixmap _function;
    bool _draw_function;
};
