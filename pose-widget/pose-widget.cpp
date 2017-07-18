/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "pose-widget.hpp"
#include "compat/util.hpp"
#include "compat/timer.hpp"
#include "compat/sleep.hpp"
#include <cmath>
#include <algorithm>
#include <QPainter>
#include <QPaintEvent>

#include <QDebug>

using namespace pose_widget_impl;

static constexpr int offset = 2;

pose_transform::pose_transform(QWidget* dst) :
    dst(dst),
    image(w+offset*2, h+offset*2, QImage::Format_ARGB32),
    image2(w+offset*2, h+offset*2, QImage::Format_ARGB32),
    fresh(false)
{
    front = QImage(QString(":/images/side1.png"));
    back = QImage(QString(":/images/side6.png"));

    image.fill(Qt::transparent);
    image2.fill(Qt::transparent);

    start();
}

pose_transform::~pose_transform()
{
    requestInterruption();
    wait();
}

void pose_widget::paintEvent(QPaintEvent* event)
{
    QPainter p(this);

    xform.with_image_lock([&](const QImage& image)
    {
        p.drawImage(event->rect(), image, QRect(offset, offset, pose_transform::w, pose_transform::h));
    });
}

void pose_transform::run()
{
    for (;;)
    {
        if (isInterruptionRequested())
            break;

        {
            lock_guard l(mtx);

            if (fresh)
                goto end;

            rotation = rotation_;
            translation = translation_;
        }

        project_quad_texture();

end:
        portable::sleep(23);
    }
}

pose_widget::pose_widget(QWidget* parent) : QWidget(parent), xform(this)
{
    rotate_sync(0,0,0, 0,0,0);
}

pose_widget::~pose_widget()
{
}

void pose_widget::rotate_async(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    bool expected = true;
    if (xform.fresh.compare_exchange_weak(expected, false))
    {
        repaint();
        xform.rotate_async(xAngle, yAngle, zAngle, x, y, z);
    }
}

template<typename F>
void pose_transform::with_rotate(F&& fun, double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    using std::sin;
    using std::cos;

    static constexpr double d2r = M_PI / 180;

    euler::euler_t euler(-zAngle * d2r, xAngle * d2r, -yAngle * d2r);
    euler::rmat r = euler::euler_to_rmat(euler);

    lock_guard l(mtx);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            rotation_(i, j) = num(r(i, j));

    translation_ = vec3(x, y, z);

    fun();
}

void pose_widget::rotate_sync(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    xform.rotate_sync(xAngle, yAngle, zAngle, x, y, z);
}

void pose_transform::rotate_async(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    with_rotate([this]() {}, xAngle, yAngle, zAngle, x, y, z);
}

void pose_transform::rotate_sync(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    with_rotate([this]() {
        rotation = rotation_;
        translation = translation_;
        project_quad_texture();
        dst->repaint();
    }, xAngle, yAngle, zAngle, x, y, z);
}

Triangle::Triangle(const vec2& p1, const vec2& p2, const vec2& p3)
{
    origin = p1;

    v0 = vec2(p3 - p1);
    v1 = vec2(p2 - p1);

    dot00 = v0.dot(v0);
    dot01 = v0.dot(v1);
    dot11 = v1.dot(v1);

    const num denom = dot00 * dot11 - dot01 * dot01;

    invDenom = 1 / denom;
}

bool Triangle::barycentric_coords(const vec2& px, vec2& uv, int& i) const
{
    i = 0;
    const vec2 v2 = px - origin;
    const num dot12 = v1.dot(v2);
    const num dot02 = v0.dot(v2);
    num u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    num v = (dot00 * dot12 - dot01 * dot02) * invDenom;
    if (!(u >= 0 && v >= 0))
    {
        return false;
    }
    if (u + v > 1)
    {
        i = 1;

        u = 1 - u;
        v = 1 - v;
    }
    uv = vec2(u, v);
    return u >= 0 && v >= 0 && u + v <= 1;
}

void pose_transform::project_quad_texture()
{
    image.fill(Qt::transparent);

    num dir;
    vec2 pt[4];
    const int sx = w - 1, sy = h - 1;
    vec2 projected[3];

    {
        const int sx_ = (sx - std::max(0, (sx - sy)/2)) * 5/9;
        const int sy_ = (sy - std::max(0, (sy - sx)/2)) * 5/9;

        static constexpr const double c = 85/100.;

        const vec3 dst_corners[] =
        {
            vec3(-sx_/2. * c, -sy_/2., 0),
            vec3(sx_/2. * c, -sy_/2., 0),
            vec3(-sx_/2. * c, sy_/2., 0),
            vec3(sx_/2. * c, sy_/2., 0.)
        };

        for (int i = 0; i < 4; i++)
            pt[i] = project(dst_corners[i]) + vec2(sx/2, sy/2);

        {
            vec3 foo[3];
            for (int i = 0; i < 3; i++)
            {
                foo[i] = project2(dst_corners[i]);
                projected[i] = project(dst_corners[i]) + vec2(sx/2, sy/2);
            }

            vec3 p1 = foo[1] - foo[0];
            vec3 p2 = foo[2] - foo[0];
            dir = p1.x() * p2.y() - p1.y() * p2.x(); // Z part of the cross product
        }
    }

    // rotation of (0, 90, 0) makes it numerically unstable
    if (std::fabs(dir) < 1e-3)
    {
        lock_guard l(mtx2);
        std::swap(image, image2);
        fresh = true;
        return;
    }

    const QImage& tex = dir < 0 ? back : front;

    Triangle t(projected[0], projected[1], projected[2]);

    const unsigned orig_pitch = tex.bytesPerLine();
    const unsigned dest_pitch = image.bytesPerLine();

    const unsigned char* restrict orig = tex.bits();
    unsigned char* restrict dest = image.bits() + offset*dest_pitch;

    const int orig_depth = tex.depth() / 8;
    const int dest_depth = image.depth() / 8;

    if (unlikely(orig_depth != 4 || dest_depth != 4))
    {
        qDebug() << "pose-widget: octopus must be saved as .png with 32 bits pixel";
        qDebug() << "pose-widget: target texture must be ARGB32";
        return;
    }

    static constexpr unsigned xmax = w, ymax = h;

    if (uv_vec.size() < xmax * ymax)
        uv_vec.resize(xmax * ymax);

    for (unsigned y = 0; y < ymax; y++)
        for (unsigned x = 0; x < xmax; x++)
        {
            uv_& restrict_ref uv = uv_vec[y * xmax + x];
            if (!t.barycentric_coords(vec2(x, y), uv.coords, uv.i))
                uv.i = -1;
        }

    const int ow = tex.width(), oh = tex.height();

    vec2 const origs[2][3] =
    {
        {
            { 0, 0 },
            { ow, 0 },
            { 0, oh },
        },
        {
            { ow, oh },
            vec2(0, oh) - vec2(ow, oh),
            vec2(ow, 0) - vec2(ow, oh),
        }
    };

    for (unsigned y = 0; y < ymax; y++)
        for (unsigned x = 0; x < xmax; x++)
        {
            uv_ const& restrict_ref uv__ = uv_vec[y * xmax + x];

            if (uv__.i != -1)
            {
                using uc = unsigned char;

                vec2 const& uv = uv__.coords;
                int const i = uv__.i;

                const float fx = origs[i][0].x()
                                 + uv.x() * origs[i][2].x()
                                 + uv.y() * origs[i][1].x();
                const float fy = origs[i][0].y()
                                 + uv.x() * origs[i][2].y()
                                 + uv.y() * origs[i][1].y();

#define BILINEAR_FILTER

#if defined BILINEAR_FILTER
                const unsigned px_ = fx + 1;
                const unsigned py_ = fy + 1;
#endif
                const unsigned px = fx;
                const unsigned py = fy;

                const unsigned orig_pos = py * orig_pitch + px * orig_depth;
#if defined BILINEAR_FILTER
                const unsigned orig_pos_ = py_ * orig_pitch + px_ * orig_depth;
                const unsigned orig_pos__ = py * orig_pitch + px_ * orig_depth;
                const unsigned orig_pos___ = py_ * orig_pitch + px * orig_depth;
#endif

                // 1, 0 -- ax_, ay
                // 0, 1 -- ax, ay_
                // 1, 1 -- ax_, ay_
                // 0, 0 -- ax, ay
                //const uc alpha = (a1 * ax + a3 * ax_) * ay + (a4 * ax + a2 * ax_) * ay_;

#if defined BILINEAR_FILTER
                const float ax_ = fx - unsigned(fx);
                const float ay_ = fy - unsigned(fy);
                const float ax = 1 - ax_;
                const float ay = 1 - ay_;
#endif

                const unsigned pos = y * dest_pitch + (x+offset) * dest_depth;

                for (int k = 0; k < 4; k++)
                {
#if defined BILINEAR_FILTER
                    const uc i = orig[orig_pos + k];
                    const uc i_ = orig[orig_pos_ + k];
                    const uc i__ = orig[orig_pos__ + k];
                    const uc i___ = orig[orig_pos___ + k];

                    dest[pos + k] = uc((i * ax + i__ * ax_) * ay + (i___ * ax + i_ * ax_) * ay_);
#else
                    dest[pos + k] = orig[orig_pos + k];
#endif
                }
            }
        }

    {
        lock_guard l2(mtx2);
        std::swap(image, image2);
        fresh = true;
    }
}

vec2 pose_transform::project(const vec3 &point)
{
    using std::fabs;

    vec3 ret = rotation * point;
    num z = std::fmax(num(.5), 1 + translation.z()/-80);
    num w_ = w, h_ = h;
    num x = w_ * translation.x() / 2 / -80;
    if (fabs(x) > w_/2)
        x = x > 0 ? w_/2 : w_/-2;
    num y = h_ * translation.y() / 2 / -80;
    if (fabs(y) > h_/2)
        y = y > 0 ? h_/2 : h_/-2;
    return vec2(z * (ret.x() + x), z * (ret.y() + y));
}

vec3 pose_transform::project2(const vec3 &point)
{
    return rotation * point;
}


template<typename F>
inline void pose_transform::with_image_lock(F&& fun)
{
    lock_guard l(mtx2);

    fun(image2);
}
