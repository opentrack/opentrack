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
#include <QImage>

namespace pose_widget_impl {

pose_widget::pose_widget(QWidget* parent) : QWidget(parent)
{
}

void pose_widget::present(double yaw, double pitch, double roll, double x, double y, double z)
{
    T = { x, y, z };
    R = { yaw, pitch, roll };

    repaint();
}

void pose_widget::paintEvent(QPaintEvent*)
{
    auto [ yaw, pitch, roll ] = R;
    auto [ x, y, z ] = T;

    const QImage& img = (std::fabs(pitch) > 90) ^ (std::fabs(yaw) > 90)
                        ? back
                        : front;

    int w = img.width(), h = img.height();

    QTransform t;

    t.translate(w*.5, h*.5);

    constexpr double z_scale = 1./250;
    constexpr double xy_scale = .0075;
    double xy = std::sqrt(w*w + h*h) * xy_scale;

    double s = clamp(.25 + -z * z_scale, .01, 2);
    t.scale(s, s);

    t.rotate(pitch, Qt::XAxis);
    t.rotate(yaw, Qt::YAxis);
    t.rotate(roll, Qt::ZAxis);

    t.translate(x * xy / s, y * xy / s);

    t.translate(w*.5, h*-.5);

    QPainter p(this);
    p.setTransform(t);
    p.drawImage(rect(), img);
}

QSize pose_widget::sizeHint() const
{
    return { 1 << 16, 1 << 16 };
}

} // ns pose_widget_impl
