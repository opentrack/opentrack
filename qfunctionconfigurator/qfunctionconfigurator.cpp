/* Copyright (c) 2011-2012 Stanislaw Halik <sthalik@misaki.pl>
 *                         Adapted to FaceTrackNoIR by Wim Vriend.
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "qfunctionconfigurator/qfunctionconfigurator.h"
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPathStroker>
#include <QPainterPath>
#include <QBrush>
#include <QFileDialog>
#include <QPen>
#include <QMessageBox>
#include <QImage>
#include <QPixmap>
#include <QTimer>
#include <QtDebug>
#include <cmath>
#include <QTabWidget>
#include <QTabBar>
#include <QFontMetrics>

static const int pointSize = 5;

QFunctionConfigurator::QFunctionConfigurator(QWidget *parent)
    : QWidget(parent)
{
    movingPoint = -1;				// Index of that same point
    _config = 0;
    _draw_background = true;
    _draw_function = true;
    update_range();
    setMouseTracking(true);
}

void QFunctionConfigurator::setConfig(FunctionConfig* config) {
    QSettings settings("opentrack");	// Registry settings (in HK_USER)
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    config->loadSettings(iniFile);
    _config = config;
    _draw_function = _draw_background = true;
    update_range();
    update();
}

void QFunctionConfigurator::saveSettings(QString settingsFile) {
    QSettings iniFile( settingsFile, QSettings::IniFormat );						// Application settings (in INI-file)

    if (_config) {
        _config->saveSettings(iniFile);
    }
}

void QFunctionConfigurator::drawBackground()
{
    if (!_config)
        return;
    _background = QPixmap(width(), height());
    QPainter painter(&_background);
    painter.fillRect(rect(), QColor::fromRgb(204, 204, 204));
    painter.setRenderHint(QPainter::Antialiasing);
    QColor bg_color(112, 154, 209);
    painter.fillRect(range, bg_color);

    QFont font;
    font.setPointSize(8);
    painter.setFont(font);
    QFontMetrics metrics(font);

    QPen pen(QColor(55, 104, 170, 127), 1, Qt::SolidLine);

    const int xstep = 10, ystep = 10;
    const int maxx = _config->maxInput();
    const int maxy = _config->maxOutput();

    // horizontal grid

    for (int i = 0; i < maxy; i += xstep)
    {
        double y = range.height() - i * c.y() + range.y();
        drawLine(&painter,
                 QPointF(range.x(), y),
                 QPointF(range.x() + range.width(), y),
                 pen);
        painter.drawText(QRectF(10,
                                y - metrics.height()/2,
                                range.left(),
                                metrics.height()),
                         QString::number(i));
    }

    {
        const int i = maxy;
        double y = range.height() - i * c.y() + range.y();
        drawLine(&painter,
                 QPointF(range.x(), y),
                 QPointF(range.x() + range.width(), y),
                 pen);
        painter.drawText(QRectF(10,
                                y - metrics.height()/2,
                                range.x() - 10,
                                metrics.height()),
                         QString::number(i));
    }

    // vertical grid

    for (int i = 0; i < maxx; i += ystep)
    {
        double x = range.x() + i * c.x();
        drawLine(&painter,
                 QPointF(x, range.y()),
                 QPointF(x, range.y() + range.height()),
                 pen);
        const QString text = QString::number(i);
        painter.drawText(QRectF(x - metrics.width(text)/2,
                                range.height() + 10 + metrics.height(),
                                metrics.width(text),
                                metrics.height()),
                         text);
    }
    {
        const int i = maxx;
        double x = range.x() + i * c.x();
        drawLine(&painter,
                 QPointF(x, range.y()),
                 QPointF(x, range.y() + range.height()),
                 pen);
        const QString text = QString::number(i);
        painter.drawText(QRectF(x - metrics.width(text)/2,
                                range.height() + 10 + metrics.height(),
                                metrics.width(text),
                                metrics.height()),
                         text);
    }
}

void QFunctionConfigurator::drawFunction()
{
    if (!_config)
        return;
    int i;
    QPointF prevPoint;
    QPointF currentPoint;

    _function = QPixmap(_background);
    QPainter painter(&_function);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QList<QPointF> points = _config->getPoints();

    for (i = 0; i < points.size(); i++) {
        currentPoint = point_to_pixel( points[i] );		// Get the next point and convert it to Widget measures
        drawPoint(&painter, currentPoint, QColor(200, 200, 210, 120));
        lastPoint = currentPoint;											// Remember which point is the rightmost in the graph
    }


    QPen pen(colBezier, 1.2, Qt::SolidLine);

    prevPoint = point_to_pixel( QPointF(0,0) );		// Start at the Axis
    double max = _config->maxInput();
    QPointF prev = point_to_pixel(QPointF(0, 0));
    const double step = 1.01;
    for (double i = 0; i < max; i += step) {
        double val = _config->getValue(i);
        QPointF cur = point_to_pixel(QPointF(i, val));
        drawLine(&painter, prev, cur, pen);
        prev = cur;
    }
    painter.restore();
}

void QFunctionConfigurator::paintEvent(QPaintEvent *e)
{
    QPointF prevPoint;
    QPointF currentPoint;
    QPointF actualPos;
    int i;

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (_draw_background) {
        drawBackground();
        _draw_background = false;
    }
    p.drawPixmap(e->rect(), _background);

    if (_draw_function) {
        drawFunction();						// Draw the Function on a Pixmap
        _draw_function = false;
    }
    p.drawPixmap(e->rect(), _function);						// Always draw the background and the function

    if (_config) {
        QPen pen(Qt::white, 1, Qt::SolidLine);
        QList<QPointF> points = _config->getPoints();
        if (movingPoint >= 0 && movingPoint < points.size()) {
            prevPoint = point_to_pixel( QPointF(0,0) );				// Start at the Axis
            for (i = 0; i < points.size(); i++) {
                currentPoint = point_to_pixel( points[i] );		// Get the next point and convert it to Widget measures
                drawLine(&p, prevPoint, currentPoint, pen);
                prevPoint = currentPoint;
            }
            pen.setWidth(1);
            pen.setColor( Qt::white );
            pen.setStyle( Qt::DashLine );
            actualPos = point_to_pixel(points[movingPoint]);
            drawLine(&p, QPoint(range.left(), actualPos.y()), QPoint(actualPos.x(), actualPos.y()), pen);
            drawLine(&p, QPoint(actualPos.x(), actualPos.y()), QPoint(actualPos.x(), range.height() + range.top()), pen);
        }

        //
        // If the Tracker is active, the 'Last Point' it requested is recorded.
        // Show that point on the graph, with some lines to assist.
        // This new feature is very handy for tweaking the curves!
        //
        if (_config->getLastPoint( currentPoint )) {
            actualPos = point_to_pixel( QPointF(fabs(currentPoint.x()), fabs(currentPoint.y())) );
            drawPoint(&p, actualPos, QColor(255, 0, 0, 120));

            pen.setWidth(1);
            pen.setColor( Qt::black );
            pen.setStyle( Qt::SolidLine );
            drawLine(&p, QPoint(range.left(), actualPos.y()), QPoint(actualPos.x(), actualPos.y()), pen);
            drawLine(&p, QPoint(actualPos.x(), actualPos.y()), QPoint(actualPos.x(), range.width()), pen);
        }

    }
}

//
// Draw the handle, to move the Bezier-curve.
//
void QFunctionConfigurator::drawPoint(QPainter *painter, const QPointF &pos, QColor colBG )
{
    painter->save();
    painter->setPen(QColor(50, 100, 120, 200));
    painter->setBrush( colBG );
    painter->drawEllipse(QRectF(pos.x() - pointSize,
                                pos.y() - pointSize,
                                pointSize*2, pointSize*2));
    painter->restore();
}

void QFunctionConfigurator::drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen pen)
{
    painter->save();
    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawLine(start, end);
    painter->restore();
}

void QFunctionConfigurator::mousePressEvent(QMouseEvent *e)
{
    if (!_config)
        return;
    QList<QPointF> points = _config->getPoints();
    if (e->button() == Qt::LeftButton) {
        bool bTouchingPoint = false;
        movingPoint = -1;
        if (_config) {
            for (int i = 0; i < points.size(); i++) {
                if ( point_within_pixel(points[i], e->pos() ) ) {
                    bTouchingPoint = true;
                    movingPoint = i;
                    timer.restart();
                    break;
                }
            }
            if (!bTouchingPoint) {
                _config->addPoint(pixel_coord_to_point(e->pos()));
                emit CurveChanged( true );
            }
        }
    }

    if (e->button() == Qt::RightButton) {
        if (_config) {
            int found_pt = -1;
            for (int i = 0; i < points.size(); i++) {
                if ( point_within_pixel(points[i], e->pos() ) ) {
                    found_pt = i;
                    break;
                }
            }

            if (found_pt != -1) {
                _config->removePoint(found_pt);
                emit CurveChanged( true );
            }
            movingPoint = -1;
        }
    }
    _draw_function = true;
    update();
}

void QFunctionConfigurator::mouseMoveEvent(QMouseEvent *e)
{
    if (!_config)
        return;
    QList<QPointF> points = _config->getPoints();
    const int refresh_delay = 50;

    if (movingPoint >= 0 && movingPoint < points.size()) {
        setCursor(Qt::ClosedHandCursor);

        if (timer.isValid() && timer.elapsed() > refresh_delay)
        {
            timer.restart();
            QPointF new_pt = pixel_coord_to_point(e->pos());
            points[movingPoint] = new_pt;
            _config->movePoint(movingPoint, new_pt);
            _draw_function = true;
            update();
        }
    }
    else {
        bool bTouchingPoint = false;
        if (_config) {
            for (int i = 0; i < points.size(); i++) {
                if ( point_within_pixel(points[i], e->pos() ) ) {
                    bTouchingPoint = true;
                }
            }
        }

        if ( bTouchingPoint ) {
            setCursor(Qt::OpenHandCursor);
        }
        else {
            setCursor(Qt::ArrowCursor);
        }
    }
}

void QFunctionConfigurator::mouseReleaseEvent(QMouseEvent *e)
{
    if (!_config)
        return;
    QList<QPointF> points = _config->getPoints();

    if (e->button() == Qt::LeftButton) {
        timer.invalidate();
        if (movingPoint >= 0 && movingPoint < points.size()) {
            emit CurveChanged( true );
            if (_config) {
                _config->movePoint(movingPoint, pixel_coord_to_point(e->pos()));
            }
        }
        setCursor(Qt::ArrowCursor);
        movingPoint = -1;
    }

    _draw_function = true;
    update();
}

bool QFunctionConfigurator::point_within_pixel(QPointF pt, QPointF pixel) const
{
    QPointF pixel2(range.x() + pt.x() * c.x(), (range.y() + range.height() - pt.y() * c.y()));
    return pixel2.x() >= pixel.x() - pointSize && pixel2.x() < pixel.x() + pointSize &&
           pixel2.y() >= pixel.y() - pointSize && pixel2.y() < pixel.y() + pointSize;
}

QPointF QFunctionConfigurator::pixel_coord_to_point(QPointF point) const
{
    if (!_config)
        return QPointF(-1, -1);

    double x = (point.x() - range.x()) / c.x();
    double y = (range.height() - point.y() + range.y()) / c.y();

    if (x < 0)
        x = 0;
    if (x > _config->maxInput())
        x = _config->maxInput();

    if (y < 0)
        y = 0;
    if (y > _config->maxOutput())
        y = _config->maxOutput();

    return QPointF(x, y);
}

QPointF QFunctionConfigurator::point_to_pixel(QPointF point) const
{
    return QPointF(range.x() + point.x() * c.x(),
                   range.y() + range.height() - point.y() * c.y());
}

void QFunctionConfigurator::setColorBezier(QColor color)
{
    colBezier = color;
    update();
}

void QFunctionConfigurator::resizeEvent(QResizeEvent *)
{
    update_range();
    repaint();
}
