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
#include <QtDesigner/QDesignerExportWidget>
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
    
    Q_PROPERTY(int maxInputEGU READ maxInputEGU WRITE setmaxInputEGU)
    Q_PROPERTY(int maxOutputEGU READ maxOutputEGU WRITE setmaxOutputEGU)
    Q_PROPERTY(int pixPerEGU_Input READ pixPerEGU_Input WRITE setpixPerEGU_Input)
    Q_PROPERTY(int pixPerEGU_Output READ pixPerEGU_Output WRITE setpixPerEGU_Output)
    Q_PROPERTY(int gridDistEGU_Input READ gridDistEGU_Input WRITE setgridDistEGU_Input)
    Q_PROPERTY(int gridDistEGU_Output READ gridDistEGU_Output WRITE setgridDistEGU_Output)

    Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier)
    Q_PROPERTY(QString stringInputEGU READ stringInputEGU WRITE setInputEGU)
    Q_PROPERTY(QString stringOutputEGU READ stringOutputEGU WRITE setOutputEGU)
    Q_PROPERTY(QString stringCaption READ stringCaption WRITE setCaption)

	// Return the current value to Designer
	int maxInputEGU() const
    {
        return MaxInput;
    }
	int maxOutputEGU() const
    {
        return MaxOutput;
    }
	int pixPerEGU_Input() const
    {
        return pPerEGU_Input;
    }
	int pixPerEGU_Output() const
    {
        return pPerEGU_Output;
    }
	int gridDistEGU_Input() const
    {
        return gDistEGU_Input;
    }
	int gridDistEGU_Output() const
    {
        return gDistEGU_Output;
    }

	QColor colorBezier() const
    {
        return colBezier;
    }
	QString stringInputEGU() const
    {
        return strInputEGU;
    }
	QString stringOutputEGU() const
    {
        return strOutputEGU;
    }
	QString stringCaption() const
    {
        return strCaption;
    }

public:
	QFunctionConfigurator(QWidget *parent = 0);
	FunctionConfig* config();

	void setConfig(FunctionConfig* config, QString settingsFile);		// Connect the FunctionConfig to the Widget.
	void loadSettings(QString settingsFile);							// Load the FunctionConfig (points) from the INI-file
	void saveSettings(QString settingsFile);							// Save the FunctionConfig (points) to the INI-file

signals:
    void CurveChanged(bool);

public slots:
    void setmaxInputEGU(int);
    void setmaxOutputEGU(int);
    void setpixPerEGU_Input(int);
    void setpixPerEGU_Output(int);
    void setgridDistEGU_Input(int);
    void setgridDistEGU_Output(int);

    void setColorBezier(QColor);
    void setInputEGU(QString);
    void setOutputEGU(QString);
    void setCaption(QString);

protected slots:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
      
protected:
	void drawBackground(const QRectF &rect);
	void drawFunction();
	void drawPoint(QPainter *painter, const QPointF &pt, QColor colBG );
	void drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen pen);
	bool markContains(const QPointF &pt, const QPointF &coord) const;
	bool withinRect( const QPointF &coord, const QRectF &rect ) const;

protected:
	virtual void resizeEvent(QResizeEvent *);

private:
	QRectF  range;														// The actual rectangle for the Bezier-curve
	QPointF lastPoint;													// The right-most point of the Function
	QPointF normalizePoint (QPointF point) const;						// Convert the graphical Point to a real-life Point
    QPointF graphicalizePoint (QPointF point) const;	// Convert the Point to a graphical Point

    int     movingPoint;
    QElapsedTimer timer;

	int MaxInput;					// Maximum input limit
	int MaxOutput;					// Maximum output limit
	int pPerEGU_Input;				// Number of pixels, per EGU of Input
	int pPerEGU_Output;				// Number of pixels, per EGU of Output
	int gDistEGU_Input;				// Distance of the grid, in EGU of Input
	int gDistEGU_Output;			// Distance of the grid, in EGU of Output

	QColor colBezier;				// Color of Bezier curve
	QString strInputEGU;			// Engineering Units input (vertical axis)
	QString strOutputEGU;			// Engineering Units output (horizontal axis)
	QString strCaption;				// Caption of the graph
	QString strSettingsFile;		// Name of last read INI-file

	bool _draw_background;			// Flag to determine if the background should be (re-)drawn on the QPixmap
	QPixmap _background;			// Image of the static parts (axis, lines, etc.)
	bool _draw_function;			// Flag to determine if the function should be (re-)drawn on the QPixmap
	QPixmap _function;				// Image of the function (static unless edited by the user)

	FunctionConfig* _config;
};

#endif // QFUNCTIONCONFIGURATOR_H
