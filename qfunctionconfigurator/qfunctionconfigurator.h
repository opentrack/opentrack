/* Copyright (c) 2011-2014 Stanislaw Halik <sthalik@misaki.pl>
 *                         Adapted to FaceTrackNoIR by Wim Vriend.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
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
    
	FunctionConfig* config();
    void setConfig(FunctionConfig* config, const QString &name);
    
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
	virtual void resizeEvent(QResizeEvent *);
private:
    void update_range() {
        if (!_config)
            return;
        
        const double w = width(), h = height();
        const double mwl = 40, mhl = 20;
        const double mwr = 15, mhr = 35;
        
        pixel_bounds = QRectF(mwl, mhl, (w - mwl - mwr), (h - mhl - mhr));
        c = QPointF(pixel_bounds.width() / _config->maxInput(), pixel_bounds.height() / _config->maxOutput());
        _draw_function = true;
        
        _background = QPixmap();
        _function = QPixmap();
    }

    QPointF pixel_coord_to_point (const QPointF& point);
    QPointF point_to_pixel (const QPointF& point);

    FunctionConfig* _config;
    
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
