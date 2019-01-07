/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "pose-widget.hpp"
#include "compat/timer.hpp"
#include "compat/sleep.hpp"
#include "compat/check-visible.hpp"
#include "compat/math.hpp"

#include <cmath>
#include <algorithm>

#include <QPainter>
#include <QtEvents>

#include <QDebug>

// XXX this needs rewriting in terms of scanline rendering -sh 20180105
// see: <https://mikro.naprvyraz.sk/docs/Coding/2/TEXTURE4.TXT>

using namespace pose_widget_impl;

pose_transform::pose_transform(QWidget* dst) :
    dst(dst),
    front(QImage{":/images/side1.png"}.convertToFormat(QImage::Format_ARGB32)),
    back(QImage{":/images/side6.png"}.convertToFormat(QImage::Format_ARGB32)),
    image(w, h, QImage::Format_ARGB32),
    image2(w, h, QImage::Format_ARGB32),
    fresh(false)
{
    image.fill(Qt::transparent);
    image2.fill(Qt::transparent);
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
        p.drawImage(event->rect(), image, QRect(0, 0, pose_transform::w, pose_transform::h));
    });

    if (!xform.isRunning())
        xform.start(QThread::LowPriority);
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

pose_widget::~pose_widget() = default;

void pose_widget::rotate_async(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    if (!check_is_visible())
        return;

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

    constexpr double d2r = M_PI / 180;

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
    with_rotate([] {}, xAngle, yAngle, zAngle, x, y, z);
}

void pose_transform::rotate_sync(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    with_rotate([this] {
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

std::pair<vec2i, vec2i> pose_transform::get_bounds(const vec2& size)
{
    const num x = size.x(), y = size.y();

    const vec3 corners[] = {
        { -x, -y, 0 },
        { x, -y, 0 },
        { -x, y, 0 },
        { x, y, 0 },
    };

    vec2 min(w-1, h-1), max(0, 0);

    for (unsigned k = 0; k < 4; k++) // NOLINT(modernize-loop-convert)
    {
        const vec2 pt = project(corners[k]) + vec2(w/2, h/2);

        min.x() = std::fmin(min.x(), pt.x());
        min.y() = std::fmin(min.y(), pt.y());

        max.x() = std::fmax(max.x(), pt.x());
        max.y() = std::fmax(max.y(), pt.y());
    }

    min.x() = clamp(min.x(), 0, w-1);
    min.y() = clamp(min.y(), 0, h-1);
    max.x() = clamp(max.x(), 0, w-1);
    max.y() = clamp(max.y(), 0, h-1);

#if 0
    {
        QPainter p(&image);
        p.drawRect(min.x(), min.y(), max.x()-min.x(), max.y()-min.y());
    }
#endif

    return std::make_pair(vec2i(iround(min.x()), iround(min.y())),
                          vec2i(iround(max.x()), iround(max.y())));
}

void pose_transform::project_quad_texture()
{
    vec3 bgcolor;
    {
        QColor bg = dst->palette().background().color();
        image.fill(bg);
        bgcolor = vec3(bg.red(), bg.green(), bg.blue());
    }

    num dir;
    vec2 pt[4];

    vec2i min, max;

    {
        constexpr double c = 85/100.;

        const int sx_ = (w - std::max(0, (w - h)/2)) * 5/9;
        const int sy_ = (h - std::max(0, (h - w)/2)) * 5/9;

        std::tie(min, max) = get_bounds(vec2(sx_/2.*c, sy_/2));

        const vec3 dst_corners[] =
        {
            { -sx_/2. * c, -sy_/2., 0 },
            { sx_/2. * c, -sy_/2., 0 },
            { -sx_/2. * c, sy_/2., 0 },
            { sx_/2. * c, sy_/2., 0 },
        };

        for (int i = 0; i < 4; i++)
            pt[i] = project(dst_corners[i]) + vec2(w/2, h/2);

        {
            vec3 foo[3];
            for (int i = 0; i < 3; i++)
                foo[i] = project2(dst_corners[i]);

            vec3 p1 = foo[1] - foo[0];
            vec3 p2 = foo[2] - foo[0];
            dir = p1.x() * p2.y() - p1.y() * p2.x(); // Z part of the cross product
        }
    }

    // rotation of (0, 90, 0) makes it numerically unstable
    if (std::fabs(dir) < 1e-3f)
    {
        lock_guard l(mtx2);
        image.swap(image2);
        fresh = true;
        return;
    }

    const QImage& tex = dir < 0 ? back : front;

    const unsigned orig_pitch = (unsigned)tex.bytesPerLine();
    const unsigned dest_pitch = (unsigned)image.bytesPerLine();

    unsigned char const* __restrict orig = tex.constBits();
    unsigned char* __restrict dest = image.bits();

    const int orig_depth = tex.depth() / 8;
    const int dest_depth = image.depth() / 8;
    constexpr unsigned const_depth = 4;

    if (unlikely(orig_depth != const_depth || dest_depth != const_depth))
    {
        qDebug() << "pose-widget: octopus must be saved as .png with 32 bits depth";
        qDebug() << "pose-widget: target texture must be ARGB32";
        return;
    }

    Triangle t(pt[0], pt[1], pt[2]);

    const vec2u dist(max.x() - min.x(), max.y() - min.y());
    unsigned len = (unsigned)(dist.x() * dist.y());

    if (uv_vec.size() < len)
        uv_vec.resize(len);

    for (int y = 0; y < dist.y(); y++)
        for (int x = 0; x < dist.x(); x++)
        {
            uv_* __restrict uv = &uv_vec[y * dist.x() + x];
            if (!t.barycentric_coords(vec2(x + min.x(), y + min.y()), uv->coords, uv->i))
                uv->i = -1;
        }

    const int ow = tex.width(), oh = tex.height();

    vec2 const origs[2][3] =
    {
        {
            { 0, 0 },
            { ow-1, 0 },
            { 0, oh-1 },
        },
        {
            { ow-1, oh-1 },
            vec2(0, oh-1) - vec2(ow-1, oh-1),
            vec2(ow-1, 0) - vec2(ow-1, oh-1),
        }
    };

    for (int y_ = 0, dy = dist.y(); y_ < dy; y_++)
    {
        for (int x_ = 0, dx = dist.x(); x_ < dx; x_++)
        {
            const int y = y_ + min.y(), x = x_ + min.x();
            const uv_* __restrict uv__ = &uv_vec[y_ * dx + x_];

            if (uv__->i != -1)
            {
                using uc = unsigned char;

                vec2 const& uv = uv__->coords;
                int const i = uv__->i;

                unsigned px = (unsigned)(origs[i][0].x() +
                                         uv.x() * origs[i][2].x() +
                                         uv.y() * origs[i][1].x());
                unsigned py = (unsigned)(origs[i][0].y() +
                                         uv.x() * origs[i][2].y() +
                                         uv.y() * origs[i][1].y());

                const unsigned orig_pos = py * orig_pitch + px * const_depth;
                const unsigned pos = y * dest_pitch + x * const_depth;

                if (orig[orig_pos + 3] == uc(255)) // alpha
                    for (int k = 0; k < 3; k++)
                        dest[pos + k] = orig[orig_pos + k];
                else
                    for (int k = 0; k < 3; k++)
                        dest[pos + k] = (unsigned char)bgcolor(k);
            }
        }
    }

    {
        lock_guard l2(mtx2);
        image.swap(image2);
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
    return { z * (ret.x() + x), z * (ret.y() + y) };
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
