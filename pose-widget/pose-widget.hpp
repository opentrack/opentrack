/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QtGlobal>
#include <QWidget>
#include <QThread>
#include <QPixmap>
#include "api/plugin-api.hpp"
#include "compat/euler.hpp"

#include <mutex>
#include <atomic>

#ifdef BUILD_POSE_WIDGET
#   define POSE_WIDGET_EXPORT Q_DECL_EXPORT
#else
#   define POSE_WIDGET_EXPORT Q_DECL_IMPORT
#endif

namespace pose_widget_impl {

using num = float;
using vec3 = Mat<num, 3, 1>;
using vec2 = Mat<num, 2, 1>;

using rmat = Mat<num, 3, 3>;

using namespace euler;

using lock_guard = std::unique_lock<std::mutex>;

class pose_widget;

class pose_transform final : private QThread
{
    pose_transform(QWidget* dst);
    ~pose_transform();

    friend class pose_widget;

    void rotate_async(double xAngle, double yAngle, double zAngle, double x, double y, double z);
    void rotate_sync(double xAngle, double yAngle, double zAngle, double x, double y, double z);

    template<typename F>
    void with_rotate(F&& fun, double xAngle, double yAngle, double zAngle, double x, double y, double z);

    void run() override;

    vec2 project(const vec3& point);
    vec3 project2(const vec3& point);
    void project_quad_texture();

    template<typename F>
    inline void with_image_lock(F&& fun);

    rmat rotation, rotation_;
    vec3 translation, translation_;

    std::mutex mtx, mtx2;

    QWidget* dst;

    QImage front, back;
    QImage image, image2;

    std::atomic<bool> fresh;

    static constexpr int w = 320, h = 240;
};

class POSE_WIDGET_EXPORT pose_widget final : public QWidget
{
public:
    pose_widget(QWidget *parent = nullptr);
    ~pose_widget();
    void rotate_async(double xAngle, double yAngle, double zAngle, double x, double y, double z);
    void rotate_sync(double xAngle, double yAngle, double zAngle, double x, double y, double z);

private:
    pose_transform xform;
    void paintEvent(QPaintEvent *event) override;
};

}

using pose_widget_impl::pose_widget;
