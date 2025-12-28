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
#include <QCheckBox>

//#define TEST
namespace pose_widget_impl {

using namespace euler;

struct OTR_POSE_WIDGET_EXPORT pose_widget final : QWidget
{
public:
    explicit pose_widget(QWidget *parent = nullptr);
    void present(double xAngle, double yAngle, double zAngle, double x, double y, double z);
    QCheckBox mirror{"Mirror", this};
private:
    void resizeEvent(QResizeEvent *event) override;
    void paintEvent(QPaintEvent*) override;

    QT_WARNING_PUSH
    QT_WARNING_DISABLE_DEPRECATED

    Pose_ R, T;
    QImage front{QImage{":/images/side1.png"}.convertToFormat(QImage::Format_ARGB32)};
    QImage back {QImage{":/images/side6.png"}.convertToFormat(QImage::Format_ARGB32)
                                             .mirrored(true,false)};
    QImage shine {QImage{front.width(), front.height(), QImage::Format_ARGB32}};
    QImage shadow{QImage{front.width(), front.height(), QImage::Format_ARGB32}};

    QT_WARNING_POP
};

}

using pose_widget = pose_widget_impl::pose_widget;
