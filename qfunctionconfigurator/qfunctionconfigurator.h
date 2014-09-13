/* Copyright (c) 2011-2012 Stanislaw Halik <sthalik@misaki.pl>
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
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"

class FTNOIR_TRACKER_BASE_EXPORT QFunctionConfigurator : public QWidget
{
	Q_OBJECT
    Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier)
    QColor colorBezier() const
    {
        return colBezier;
    }
public:
	QFunctionConfigurator(QWidget *parent = 0);
	FunctionConfig* config();

    void setConfig(FunctionConfig* config);
    void saveSettings(QString settingsFile);

signals:
    void CurveChanged(bool);

public slots:
    void setColorBezier(QColor);
protected slots:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
      
protected:
    void drawBackground();
	void drawFunction();
	void drawPoint(QPainter *painter, const QPointF &pt, QColor colBG );
	void drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen pen);
    bool point_within_pixel(QPointF pt, QPointF pixel) const;

protected:
	virtual void resizeEvent(QResizeEvent *);

private:
    void update_range() {
        if (!_config)
            return;
        double w = width(), h = height();
        const double mwl = 40, mhl = 20;
        const double mwr = 15, mhr = 35;
        range = QRectF(mwl, mhl, (w - mwl - mwr), (h - mhl - mhr));
        c = QPointF(range.width() / _config->maxInput(), range.height() / _config->maxOutput());
        _draw_function = _draw_background = true;
    }

	QRectF  range;
	QPointF lastPoint;
    QPointF pixel_coord_to_point (QPointF point) const;
    QPointF point_to_pixel (QPointF point) const;

    int     movingPoint;
    QElapsedTimer timer;
    QPointF c;

	QColor colBezier;

	bool _draw_background;
	QPixmap _background;
	bool _draw_function;
	QPixmap _function;

	FunctionConfig* _config;
};
