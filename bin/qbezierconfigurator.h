#ifndef QBEZIERCONFIGURATOR_H
#define QBEZIERCONFIGURATOR_H

#include <QtGui>
#include <QtDesigner/QDesignerExportWidget>
#include <QPointF>

class QDESIGNER_WIDGET_EXPORT QBezierConfigurator : public QWidget
{
	Q_OBJECT
    Q_PROPERTY(int maxInputEGU READ maxInputEGU WRITE setmaxInputEGU);
    Q_PROPERTY(int maxOutputEGU READ maxOutputEGU WRITE setmaxOutputEGU);
    Q_PROPERTY(int pixPerEGU READ pixPerEGU WRITE setpixPerEGU);

	Q_PROPERTY(QColor colorBezier READ colorBezier WRITE setColorBezier);
    Q_PROPERTY(QColor colorBackground READ colorBackground WRITE setColorBackground);
    Q_PROPERTY(QString stringInputEGU READ stringInputEGU WRITE setInputEGU);
    Q_PROPERTY(QString stringOutputEGU READ stringOutputEGU WRITE setOutputEGU);
    Q_PROPERTY(QString stringCaption READ stringCaption WRITE setCaption);

	// Return the current value to Designer
	int maxInputEGU() const
    {
        return MaxInput;
    }
	int maxOutputEGU() const
    {
        return MaxOutput;
    }
	int pixPerEGU() const
    {
        return pPerEGU;
    }

	// Return the current color to Designer
	QColor colorBezier() const
    {
        return colBezier;
    }
	// Return the current color to Designer
	QColor colorBackground() const
    {
        return colBackground;
    }
	// Return the current string to Designer
	QString stringInputEGU() const
    {
        return strInputEGU;
    }
	// Return the current string to Designer
	QString stringOutputEGU() const
    {
        return strOutputEGU;
    }
	// Return the current string to Designer
	QString stringCaption() const
    {
        return strCaption;
    }

public:
	QBezierConfigurator(QWidget *parent = 0);
	~QBezierConfigurator();

signals:
    void valueNeutralZoneChanged(int);
    void BezierCurveChanged(bool);

public slots:
    void setmaxInputEGU(int);
    void setmaxOutputEGU(int);
    void setpixPerEGU(int);

    QPointF getPointOne();
    QPointF getPointTwo();
    QPointF getPointThree();
    QPointF getPointFour();

	void setPointOne(QPointF);
    void setPointTwo(QPointF);
    void setPointThree(QPointF);
    void setPointFour(QPointF);

	void setNeutralZone(int zone);

    void setColorBezier(QColor);
    void setColorBackground(QColor);
    void setInputEGU(QString);
    void setOutputEGU(QString);
    void setCaption(QString);

protected slots:
	void paintEvent(QPaintEvent *e);
	void mousePressEvent(QMouseEvent *e);
	void mouseMoveEvent(QMouseEvent *e);
	void mouseReleaseEvent(QMouseEvent *e);
      
protected:
	void drawBackground(QPainter *painter, const QRectF &rect);
	void drawPoint(QPainter *painter, const QPointF &pt);
	void drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen pen);
	bool markContains(const QPointF &pt, const QPointF &coord) const;
	bool withinRange( const QPointF &coord ) const;

protected:
	virtual void resizeEvent(QResizeEvent *);

private:
	QRectF  range;										// The actual rectangle for the Bezier-curve
	QPointF one, two, three, four;						// The four points, that define the curve
	QPointF normalizePoint (QPointF point) const;		// Convert the graphical Point to a real-life Point
	QPointF graphicalizePoint (QPointF point) const;	// Convert the Point to a graphical Point

	QPointF mouseStart;
	QPointF *moving;
	int     movingPoint;

	int MaxInput;					// Maximum input limit
	int MaxOutput;					// Maximum output limit
	int pPerEGU;					// Number of pixels, per EGU

	QColor colBezier;				// Color of Bezier curve
	QColor colBackground;			// Color of widget background
	QString strInputEGU;			// Engineering Units input (vertical axis)
	QString strOutputEGU;			// Engineering Units output (horizontal axis)
	QString strCaption;				// Caption of the graph

};

#endif // QBEZIERCONFIGURATOR_H
