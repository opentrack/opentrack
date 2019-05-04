/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "api/plugin-api.hpp"
#include "compat/euler.hpp"

#include "export.hpp"

#include <QWidget>
#include <QImage>

namespace pose_widget_impl {

using namespace euler;

struct OTR_POSE_WIDGET_EXPORT pose_widget final : QWidget
{
public:
    pose_widget(QWidget *parent = nullptr);
    void present(double xAngle, double yAngle, double zAngle, double x, double y, double z);

private:
    void paintEvent(QPaintEvent*) override;

    Pose_ R, T;
    QImage front{QImage{":/images/side1.png"}.convertToFormat(QImage::Format_ARGB32)};
    QImage back{QImage{":/images/side6.png"}.convertToFormat(QImage::Format_ARGB32)};
};

}

using pose_widget = pose_widget_impl::pose_widget;
