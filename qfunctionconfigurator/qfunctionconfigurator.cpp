/* Copyright (c) 2011-2014 Stanislaw Halik <sthalik@misaki.pl>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

// Adapted to FaceTrackNoIR by Wim Vriend.

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

QFunctionConfigurator::QFunctionConfigurator(QWidget *parent) :
    QWidget(parent),
    _config(nullptr),
    moving_control_point_idx(-1),
    _draw_function(true)
{
    update_range();
    setMouseTracking(true);
}

void QFunctionConfigurator::setConfig(FunctionConfig* config, const QString& name) {
    QSettings settings("opentrack");
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );

    config->loadSettings(iniFile, name);
    _config = config;
    _draw_function = true;
    update_range();
    update();
}

void QFunctionConfigurator::saveSettings(QString settingsFile, const QString& name) {
    QSettings iniFile(settingsFile, QSettings::IniFormat);

    if (_config)
        _config->saveSettings(iniFile, name);
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
    painter.fillRect(pixel_bounds, bg_color);

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
        double y = pixel_bounds.height() - i * c.y() + pixel_bounds.y();
        drawLine(&painter,
                 QPointF(pixel_bounds.x(), y),
                 QPointF(pixel_bounds.x() + pixel_bounds.width(), y),
                 pen);
        painter.drawText(QRectF(10,
                                y - metrics.height()/2,
                                pixel_bounds.left(),
                                metrics.height()),
                         QString::number(i));
    }

    {
        const int i = maxy;
        double y = pixel_bounds.height() - i * c.y() + pixel_bounds.y();
        drawLine(&painter,
                 QPointF(pixel_bounds.x(), y),
                 QPointF(pixel_bounds.x() + pixel_bounds.width(), y),
                 pen);
        painter.drawText(QRectF(10,
                                y - metrics.height()/2,
                                pixel_bounds.x() - 10,
                                metrics.height()),
                         QString::number(i));
    }

    // vertical grid
    for (int i = 0; i < maxx; i += ystep)
    {
        double x = pixel_bounds.x() + i * c.x();
        drawLine(&painter,
                 QPointF(x, pixel_bounds.y()),
                 QPointF(x, pixel_bounds.y() + pixel_bounds.height()),
                 pen);
        const QString text = QString::number(i);
        painter.drawText(QRectF(x - metrics.width(text)/2,
                                pixel_bounds.height() + 10 + metrics.height(),
                                metrics.width(text),
                                metrics.height()),
                         text);
    }
    {
        const int i = maxx;
        double x = pixel_bounds.x() + i * c.x();
        drawLine(&painter,
                 QPointF(x, pixel_bounds.y()),
                 QPointF(x, pixel_bounds.y() + pixel_bounds.height()),
                 pen);
        const QString text = QString::number(i);
        painter.drawText(QRectF(x - metrics.width(text)/2,
                                pixel_bounds.height() + 10 + metrics.height(),
                                metrics.width(text),
                                metrics.height()),
                         text);
    }
}

void QFunctionConfigurator::drawFunction()
{
    if (!_config)
        return;

    _function = QPixmap(_background);
    QPainter painter(&_function);

    painter.save();
    painter.setRenderHint(QPainter::Antialiasing, true);

    QList<QPointF> points = _config->getPoints();

    for (int i = 0; i < points.size(); i++) {
        drawPoint(&painter,
                  point_to_pixel(points[i]),
                  QColor(200, 200, 210, 120));
    }

    QPen pen(spline_color, 1.2, Qt::SolidLine);

    static const constexpr double step = 1.02;
    const double max = _config->maxInput();
    
    QPointF prev = point_to_pixel(QPointF(0, 0));
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
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    if (_background.isNull())
        drawBackground();
    p.drawPixmap(e->rect(), _background);

    if (_draw_function) {
        _draw_function = false;
        drawFunction();
    }
    p.drawPixmap(e->rect(), _function);

    if (_config) {
        QPen pen(Qt::white, 1, Qt::SolidLine);
        QList<QPointF> points = _config->getPoints();
        if (moving_control_point_idx >= 0 && moving_control_point_idx < points.size()) {
            QPointF prev;
            for (int i = 0; i < points.size(); i++) {
                auto tmp = point_to_pixel(points[i]);
                drawLine(&p, prev, tmp, pen);
                prev = tmp;
            }
            pen.setWidth(1);
            pen.setColor( Qt::white );
            pen.setStyle( Qt::DashLine );
            QPointF pixel_pos = point_to_pixel(points[moving_control_point_idx]);
            drawLine(&p, QPoint(pixel_bounds.left(), pixel_pos.y()), QPoint(pixel_pos.x(), pixel_pos.y()), pen);
            drawLine(&p, QPoint(pixel_pos.x(), pixel_pos.y()), QPoint(pixel_pos.x(), pixel_bounds.height() + pixel_bounds.top()), pen);
        }

        // If the Tracker is active, the 'Last Point' it requested is recorded.
        // Show that point on the graph, with some lines to assist.
        // This new feature is very handy for tweaking the curves!
        QPointF last;
        if (_config->getLastPoint(last)) {
            QPointF pixel_pos = point_to_pixel( QPointF(fabs(last.x()), fabs(last.y())) );
            drawPoint(&p, pixel_pos, QColor(255, 0, 0, 120));

            pen.setWidth(1);
            pen.setColor( Qt::black );
            pen.setStyle( Qt::SolidLine );
            drawLine(&p, QPoint(pixel_bounds.left(), pixel_pos.y()), QPoint(pixel_pos.x(), pixel_pos.y()), pen);
            drawLine(&p, QPoint(pixel_pos.x(), pixel_pos.y()), QPoint(pixel_pos.x(), pixel_bounds.width()), pen);
        }
    }
}

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

void QFunctionConfigurator::drawLine(QPainter *painter, const QPointF &start, const QPointF &end, QPen &pen)
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
        moving_control_point_idx = -1;
        if (_config) {
            for (int i = 0; i < points.size(); i++) {
                if ( point_within_pixel(points[i], e->pos() ) ) {
                    bTouchingPoint = true;
                    moving_control_point_idx = i;
                    timer.restart();
                    break;
                }
            }
            if (!bTouchingPoint) {
                _config->addPoint(pixel_coord_to_point(e->pos()));
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
            }
            moving_control_point_idx = -1;
        }
    }
    _draw_function = true;
    update();
}

void QFunctionConfigurator::mouseMoveEvent(QMouseEvent *e)
{
    if (!_config)
        return;
    
    static const constexpr int min_refresh_delay = 25;
    
    if (timer.isValid() && timer.elapsed() < min_refresh_delay)
        return;
    
    static const constexpr int refresh_delay = 50;
    QList<QPointF> points = _config->getPoints();

    if (moving_control_point_idx >= 0 && moving_control_point_idx < points.size()) {
        setCursor(Qt::ClosedHandCursor);

        if (timer.isValid() && timer.elapsed() > refresh_delay)
        {
            timer.restart();
            QPointF new_pt = pixel_coord_to_point(e->pos());
            points[moving_control_point_idx] = new_pt;
            _config->movePoint(moving_control_point_idx, new_pt);
            _draw_function = true;
            update();
        }
    }
    else {
        bool bTouchingPoint = false;
        for (int i = 0; i < points.size(); i++) {
            if ( point_within_pixel(points[i], e->pos() ) ) {
                bTouchingPoint = true;
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
        if (moving_control_point_idx >= 0 && moving_control_point_idx < points.size()) {
            if (_config) {
                _config->movePoint(moving_control_point_idx, pixel_coord_to_point(e->pos()));
            }
        }
        setCursor(Qt::ArrowCursor);
        moving_control_point_idx = -1;
    }

    _draw_function = true;
    update();
}

bool QFunctionConfigurator::point_within_pixel(const QPointF &pt, const QPointF &pixel)
{
    QPointF pixel2(pixel_bounds.x() + pt.x() * c.x(),
                   (pixel_bounds.y() + pixel_bounds.height() - pt.y() * c.y()));
    return pixel2.x() >= pixel.x() - pointSize && pixel2.x() < pixel.x() + pointSize &&
           pixel2.y() >= pixel.y() - pointSize && pixel2.y() < pixel.y() + pointSize;
}

QPointF QFunctionConfigurator::pixel_coord_to_point(const QPointF& point)
{
    if (!_config)
        return QPointF(-1, -1);

    double x = (point.x() - pixel_bounds.x()) / c.x();
    double y = (pixel_bounds.height() - point.y() + pixel_bounds.y()) / c.y();

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

QPointF QFunctionConfigurator::point_to_pixel(const QPointF& point)
{
    return QPointF(pixel_bounds.x() + point.x() * c.x(),
                   pixel_bounds.y() + pixel_bounds.height() - point.y() * c.y());
}

void QFunctionConfigurator::resizeEvent(QResizeEvent *)
{
    update_range();
    repaint();
}
