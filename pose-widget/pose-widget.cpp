/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "pose-widget.hpp"
#include "compat/check-visible.hpp"
#include "compat/math.hpp"

#include <QPainter>
#include <QtEvents>

#include <QDebug>
#include <QQuaternion>
#include <QMatrix4x4>

namespace pose_widget_impl {

pose_widget::pose_widget(QWidget* parent) : QWidget(parent)
{
    QPainter p;
#ifdef TEST
    //draw rectangle frame around of Octopus, only if TEST defined
    p.begin(&front);
    p.setPen(QPen(Qt::red, 3, Qt::SolidLine));
    p.drawRect(0, 0, front.width()-1, front.height()-1);
    p.end();

    p.begin(&back);
    p.setPen(QPen(Qt::darkGreen, 3, Qt::SolidLine));
    p.drawRect(0, 0, back.width()-1, back.height()-1);
    p.end();
#endif

    //draw Octopus shine
    shine.fill(QColor(255,255,255));
    p.begin(&shine);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(QPointF(0,0), front);		
    p.end();

    //draw Octopus shadow
    shadow.fill(QColor(0,0,0));
    p.begin(&shadow);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.drawImage(QPointF(0,0), front);		
    p.end();
}

void pose_widget::present(double yaw, double pitch, double roll, double x, double y, double z)
{
    T = { x, y, z };
    R = { yaw, pitch, roll };

    repaint();
}

void pose_widget::resizeEvent(QResizeEvent *event)
{
    // adapt to widget size
    float w = event->size().width();
    float h = event->size().height();

    // draw axes
    QImage background(QImage(w, h, QImage::Format_ARGB32));
    QPainter p(&background);
    p.setPen(QPen(Qt::gray, 1, Qt::SolidLine));
    p.drawLine(0.5*w,   0  , 0.5*w,   h  );
    p.drawLine(  0  , 0.5*h,   w  , 0.5*h);

    // set AutoFillBackground
    QPalette palette;
    palette.setBrush(this->backgroundRole(), QBrush(background));
    setPalette(palette);
    setAutoFillBackground(true);

    // move the mirror checkbox in the lower right corner of the widget
    mirror.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    mirror.move(w - mirror.width(), h - mirror.height());
}

void pose_widget::paintEvent(QPaintEvent*)
{
    // widget settings:
    constexpr float scale  =  0.5;   // scale of Octopus height, when x = y = z = 0.0
    constexpr float XYZmax = 50.0;   // -XYZmax < x,y,z < +XYZmax (offset the Octopus by one body)
    constexpr float Kz     =  0.25;  // Z scale change limit (simulate camera focus length)

    // get a local copy of input data
    auto [ yaw, pitch, roll ] = R;
    auto [ x, y, z ] = T;

    QPainter p(this);
    #ifdef TEST
    // use antialiasing for correct frame around the Octopus, only if TEST defined
    p.setRenderHint(QPainter::Antialiasing, true);
    #endif

    // check mirror state
    if   (mirror.checkState() == Qt::Checked) x = -x;
    else { yaw = -yaw; roll = -roll; }
    y = -y;

    // rotations
    QQuaternion q = QQuaternion::fromEulerAngles(pitch,  yaw,  roll);
    QMatrix4x4  m = QMatrix4x4(q.toRotationMatrix());

    // x and y positions
    const float Kxy = (float)front.height() / XYZmax;
    QVector3D v(Kxy*x, Kxy*y, 0.0);
    v = m.transposed().map(v);
    m.translate(v);

    // perspective projection to x-y plane
    QTransform t = m.toTransform(1024).translate(-.5 * front.width(), -.5 * front.height());

    // z position by setViewport 
    const float mz = scale * height()/front.height()/exp(1.0) * exp(1.0 - z * (Kz/XYZmax)); 
    p.setViewport(QRect(.5 * width(), .5 * height(), width()*mz, height()*mz));
 
    // define forward or backward side by cross product of mapped x and y axes
    QPointF point0 =  t.map(QPointF(0, 0));
    QPointF x_dir  = (t.map(QPointF(1, 0)) -= point0);
    QPointF y_dir  = (t.map(QPointF(0, 1)) -= point0);
    const bool  forward  =  x_dir.ry()*y_dir.rx() - x_dir.rx()*y_dir.ry() < 0 ? true : false;

    // draw red or green Octopus
    p.setTransform(t);
    p.drawImage(QPointF(0,0), forward ? front : back);

    // top lighting simulation
    const float alpha = sin(pitch * M_PI / 180.0);
    p.setOpacity(0.333 * fabs(alpha));
    p.drawImage(QPointF(0,0), forward == (alpha >= 0.0) ? shine : shadow);
}

QSize pose_widget::sizeHint() const
{
    return { 1 << 16, 1 << 16 };
}

} // ns pose_widget_impl
