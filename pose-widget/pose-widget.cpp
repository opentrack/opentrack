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

pose_transform::pose_transform(QWidget* dst) :
    dst(dst),
    image(w, h, QImage::Format_ARGB32_Premultiplied),
    image2(w, h, QImage::Format_ARGB32_Premultiplied),
    width(w), height(h),
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
    xform.with_image_lock([&](const QImage& image) {
        p.drawImage(event->rect(), image);
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
        portable::sleep(9);
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

class Triangle {
    num dot00, dot01, dot11, invDenom;
    vec2 v0, v1, origin;
public:
    Triangle(const vec2& p1, const vec2& p2, const vec2& p3);
    bool barycentric_coords(const vec2& px, vec2& uv, int& i) const;
};

inline vec3 pose_transform::normal(const vec3& p1, const vec3& p2, const vec3& p3)
{
    using std::sqrt;

    vec3 u = p2 - p1;
    vec3 v = p3 - p1;

    vec3 tmp = u.cross(v);

    num i = 1/sqrt(tmp.dot(tmp));

    return tmp * i;
}

Triangle::Triangle(const vec2& p1, const vec2& p2, const vec2& p3)
{
    using std::fabs;

    origin = p1;

    v0 = vec2(p3 - p1);
    v1 = vec2(p2 - p1);

    dot00 = v0.dot(v0);
    dot01 = v0.dot(v1);
    dot11 = v1.dot(v1);

    const num denom = dot00 * dot11 - dot01 * dot01;

    if (fabs(denom) < num(1e3))
    {
        // for perpendicular plane, ensure u and v don't come out right
        // this is done here to avoid branching below, in a hot loop
        invDenom = 0;
        dot00 = dot01 = dot11 = 0;
        v0 = v1 = vec2(0, 0);
    }
    else
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
        return false;
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
    num dir;
    vec2 pt[4];
    const int sx = width - 1, sy = height - 1;
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

        vec3 normal1(0, 0, 1);
        vec3 normal2;
        {
            vec3 foo[3];
            for (int i = 0; i < 3; i++)
            {
                foo[i] = project2(dst_corners[i]);
                projected[i] = project(dst_corners[i]) + vec2(sx/2, sy/2);
            }
            normal2 = normal(foo[0], foo[1], foo[2]);
        }
        dir = normal1.dot(normal2);
    }

    const QImage& tex = dir < 0 ? back : front;
    const int ow = tex.width(), oh = tex.height();

    vec2 origs[2][3] =
    {
        {
            vec2(0, 0),
            vec2(ow-1, 0),
            vec2(0, oh-1)
        },
        {
            vec2(ow-1, oh-1),
            vec2(0, oh-1) - vec2(ow-1, oh-1),
            vec2(ow-1, 0) - vec2(ow-1, oh-1),
        }
    };

    Triangle t(projected[0], projected[1], projected[2]);

    const unsigned orig_pitch = tex.bytesPerLine();
    const unsigned dest_pitch = image.bytesPerLine();

    const unsigned char* orig = tex.bits();
    unsigned char* dest = image.bits();

    const int orig_depth = tex.depth() / 8;
    const int dest_depth = image.depth() / 8;

    /* image breakage? */
    if (orig_depth != 4)
    {
        qDebug() << "pose-widget: octopus must be saved as .png with 32 bits pixel";
        return;
    }

    if (dest_depth != 4)
    {
        qDebug() << "pose-widget: target texture must be ARGB32";
        return;
    }

    const vec3 next = rotation * vec3(1, 1, 0);

    for (int y = 1; y < sy; y++)
        for (int x = 1; x < sx; x++)
        {
            vec2 pos(x, y);
            vec2 uv;
            int i;
            if (t.barycentric_coords(pos, uv, i))
            {
                const float fx = origs[i][0].x()
                                 + uv.x() * origs[i][2].x()
                                 + uv.y() * origs[i][1].x();
                const float fy = origs[i][0].y()
                                 + uv.x() * origs[i][2].y()
                                 + uv.y() * origs[i][1].y();

                const int px_ = fx + next.x();
                const int py_ = fy + next.y();
                const int px = fx;
                const int py = fy;
                const float ax_ = fx - px;
                const float ay_ = fy - py;
                const float ax = 1 - ax_;
                const float ay = 1 - ay_;

                // 0, 0 -- ax, ay
                const unsigned orig_pos = py * orig_pitch + px * orig_depth;
                const unsigned r = orig[orig_pos + 2];
                const unsigned g = orig[orig_pos + 1];
                const unsigned b = orig[orig_pos + 0];

                // 1, 1 -- ax_, ay_
                const unsigned orig_pos_ = py_ * orig_pitch + px_ * orig_depth;
                const unsigned r_ = orig[orig_pos_ + 2];
                const unsigned g_ = orig[orig_pos_ + 1];
                const unsigned b_ = orig[orig_pos_ + 0];

                // 1, 0 -- ax_, ay
                const unsigned orig_pos__ = py * orig_pitch + px_ * orig_depth;
                const unsigned r__ = orig[orig_pos__ + 2];
                const unsigned g__ = orig[orig_pos__ + 1];
                const unsigned b__ = orig[orig_pos__ + 0];

                // 0, 1 -- ax, ay_
                const unsigned orig_pos___ = py_ * orig_pitch + px * orig_depth;
                const unsigned r___ = orig[orig_pos___ + 2];
                const unsigned g___ = orig[orig_pos___ + 1];
                const unsigned b___ = orig[orig_pos___ + 0];

                const unsigned a1 = orig[orig_pos + 3];
                const unsigned a2 = orig[orig_pos_ + 3];
                const unsigned a3 = orig[orig_pos__ + 3];
                const unsigned a4 = orig[orig_pos___ + 3];

                const unsigned pos = y * dest_pitch + x * dest_depth;

                const unsigned alpha = (a1 * ax + a3 * ax_) * ay + (a4 * ax + a2 * ax_) * ay_;

                dest[pos + 2] = unsigned((r * ax + r__ * ax_) * ay + (r___ * ax + r_ * ax_) * ay_)*alpha / 255;
                dest[pos + 1] = unsigned((g * ax + g__ * ax_) * ay + (g___ * ax + g_ * ax_) * ay_)*alpha / 255;
                dest[pos + 0] = unsigned((b * ax + b__ * ax_) * ay + (b___ * ax + b_ * ax_) * ay_)*alpha / 255;

                dest[pos + 3] = alpha;
            }
        }

    {
        lock_guard l2(mtx2);
        image2.fill(Qt::transparent);
        std::swap(image, image2);
        fresh = true;
    }
}

vec2 pose_transform::project(const vec3 &point)
{
    using std::fabs;

    vec3 ret = rotation * point;
    num z = std::max<num>(.5, 1 + translation.z()/-80);
    num w = width, h = height;
    num x = w * translation.x() / 2 / -80;
    if (fabs(x) > w/2)
        x = x > 0 ? w/2 : w/-2;
    num y = h * translation.y() / 2 / -80;
    if (fabs(y) > h/2)
        y = y > 0 ? h/2 : h/-2;
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
