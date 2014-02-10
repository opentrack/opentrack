/* Copyright (c) 2011-2012 Stanislaw Halik <sthalik@misaki.pl>
 *                         Adapted to FaceTrackNoIR by Wim Vriend.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#ifndef QFUNCTIONCONFIGURATOR_H
#define QFUNCTIONCONFIGURATOR_H

#include <QWidget>
#include <QtGui>
#include <QPointF>
#include <QElapsedTimer>
#include "qfunctionconfigurator/functionconfig.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"

//
// The FunctionConfigurator Widget is used to display and configure a function (curve).
// The Function is used by FaceTrackNoIR to 'translate' the actual head-pose to the virtual headpose. Every axis is configured by a separate Function.
//
// The Function is coded in a separate Class and can exists, without the Widget. When the widget is displayed (therefore 'created'), the Function can be attached to the
// Widget and the Widget used to change the Function.
//

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

	QRectF  range;														// The actual rectangle for the Bezier-curve
	QPointF lastPoint;													// The right-most point of the Function
    QPointF pixel_coord_to_point (QPointF point) const;						// Convert the graphical Point to a real-life Point
    QPointF point_to_pixel (QPointF point) const;	// Convert the Point to a graphical Point

    int     movingPoint;
    QElapsedTimer timer;
    QPointF c;

	QColor colBezier;				// Color of Bezier curve

	bool _draw_background;			// Flag to determine if the background should be (re-)drawn on the QPixmap
	QPixmap _background;			// Image of the static parts (axis, lines, etc.)
	bool _draw_function;			// Flag to determine if the function should be (re-)drawn on the QPixmap
	QPixmap _function;				// Image of the function (static unless edited by the user)

	FunctionConfig* _config;
};

#endif // QFUNCTIONCONFIGURATOR_H
