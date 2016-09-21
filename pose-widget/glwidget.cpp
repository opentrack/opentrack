/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "glwidget.h"
#include "api/is-window-visible.hpp"
#include <cmath>
#include <algorithm>
#include <QPainter>
#include <QPaintEvent>

#include <QDebug>

GLWidget::GLWidget(QWidget *parent) : QWidget(parent), visible(true)
{
    Q_INIT_RESOURCE(posewidget);

    front = QImage(QString(":/images/side1.png"));
    back = QImage(QString(":/images/side6.png"));
    rotateBy_real(0, 0, 0, 0, 0, 0);
}

GLWidget::~GLWidget()
{
}

void GLWidget::paintEvent (QPaintEvent * event)
{
    QPainter p(this);
    project_quad_texture();
    p.drawImage(event->rect(), image);
}

void GLWidget::rotateBy(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    if (visible_timer.elapsed_ms() > 250)
    {
        visible = is_window_visible(this);
        visible_timer.start();
    }

    if (visible)
        rotateBy_real(xAngle, yAngle, zAngle, x, y, z);
}

void GLWidget::rotateBy_real(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{
    using std::sin;
    using std::cos;

    static constexpr double d2r = M_PI / 180;

    translation = vec3(x, y, z);

    euler::euler_t euler(-zAngle * d2r, xAngle * d2r, -yAngle * d2r);
    euler::rmat r = euler::euler_to_rmat(euler);

    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            rotation(i, j) = num(r(i, j));

    update();
}

class Triangle {
    using num = GLWidget::num;
    using vec2 = GLWidget::vec2;
    using vec3 = GLWidget::vec3;
public:
    Triangle(const vec2& p1, const vec2& p2, const vec2& p3);
    bool barycentric_coords(const vec2& px, vec2& uv, int& i) const;

private:
    num dot00, dot01, dot11, invDenom;
    vec2 v0, v1, origin;
};

inline GLWidget::vec3 GLWidget::normal(const vec3& p1, const vec3& p2, const vec3& p3)
{
    using std::sqrt;

    vec3 u = p2 - p1;
    vec3 v = p3 - p1;

    vec3 tmp = u.cross(v);

    num i = 1/sqrt(tmp.dot(tmp));

    return tmp * i;
}

Triangle::Triangle(const Triangle::vec2& p1, const Triangle::vec2& p2, const Triangle::vec2& p3)
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

bool Triangle::barycentric_coords(const Triangle::vec2& px, Triangle::vec2& uv, int& i) const
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

void GLWidget::project_quad_texture()
{
    const int sx = width(), sy = height();
    const int ow = front.width(), oh = front.height();
    const vec3 corners[] = {
        vec3(-ow/2., -oh/2, 0),
        vec3(ow/2, -oh/2, 0),
        vec3(-ow/2, oh/2, 0),
        vec3(ow/2, oh/2, 0.)
    };

    vec2 pt[4];
    vec2 sz((sx-ow)/2, (sy-oh)/2);
    for (int i = 0; i < 4; i++)
        pt[i] = project(corners[i]) + vec2(sx/2, sy/2);

    vec3 normal1(0, 0, 1);
    vec3 normal2;
    {
        vec3 foo[3];
        for (int i = 0; i < 3; i++)
            foo[i] = project2(corners[i]);
        normal2 = normal(foo[0], foo[1], foo[2]);
    }

    num dir = normal1.dot(normal2);

    QImage& tex = dir < 0 ? back : front;

    QImage texture(QSize(sx, sy), QImage::Format_RGBA8888);
    QColor bgColor = palette().color(QPalette::Current, QPalette::Window);
    texture.fill(bgColor);

    const vec2 projected[2][3] =
    {
        { pt[0], pt[1], pt[2] },
        { pt[3], pt[1], pt[2] }
    };

    const vec2 origs[2][3] =
    {
        {
            vec2(0, 0),
            vec2(ow-1, 0),
            vec2(0, oh-1) },
        {
            vec2(ow-1, oh-1),
            vec2(0, oh-1),
            vec2(ow-1, 0)
        }
    };

    const Triangle triangles[2] =
    {
        Triangle(projected[0][0], projected[0][1], projected[0][2]),
        Triangle(projected[1][0], projected[1][1], projected[1][2])
    };

    const int orig_pitch = tex.bytesPerLine();
    const int dest_pitch = texture.bytesPerLine();

    const unsigned char* orig = tex.bits();
    unsigned char* dest = texture.bits();

    const int orig_depth = tex.depth() / 8;
    const int dest_depth = texture.depth() / 8;

    /* image breakage? */
    if (orig_depth < 3)
        return;

    for (int y = 0; y < sy; y++)
        for (int x = 0; x < sx; x++)
        {
            vec2 pos(x, y);
            vec2 uv;
            int i;
            if (triangles[0].barycentric_coords(pos, uv, i))
            {
                const float fx = origs[i][0].x()
                                 + uv.x() * (origs[i][2].x() - origs[i][0].x())
                                 + uv.y() * (origs[i][1].x() - origs[i][0].x());
                const float fy = origs[i][0].y()
                                 + uv.x() * (origs[i][2].y() - origs[i][0].y())
                                 + uv.y() * (origs[i][1].y() - origs[i][0].y());

                const int px_ = fx + .5f;
                const int py_ = fy + .5f;
                const int px = fx;
                const int py = fy;
                const float ax_ = fx - px;
                const float ay_ = fy - py;
                const float ax = 1.f - ax_;
                const float ay = 1.f - ay_;

                // 0, 0 -- ax, ay
                const int orig_pos = py * orig_pitch + px * orig_depth;
                const unsigned char r = orig[orig_pos + 2];
                const unsigned char g = orig[orig_pos + 1];
                const unsigned char b = orig[orig_pos + 0];

                // 1, 1 -- ax_, ay_
                const int orig_pos_ = py_ * orig_pitch + px_ * orig_depth;
                const unsigned char r_ = orig[orig_pos_ + 2];
                const unsigned char g_ = orig[orig_pos_ + 1];
                const unsigned char b_ = orig[orig_pos_ + 0];

                // 1, 0 -- ax_, ay
                const int orig_pos__ = py * orig_pitch + px_ * orig_depth;
                const unsigned char r__ = orig[orig_pos__ + 2];
                const unsigned char g__ = orig[orig_pos__ + 1];
                const unsigned char b__ = orig[orig_pos__ + 0];

                // 0, 1 -- ax, ay_
                const int orig_pos___ = py_ * orig_pitch + px * orig_depth;
                const unsigned char r___ = orig[orig_pos___ + 2];
                const unsigned char g___ = orig[orig_pos___ + 1];
                const unsigned char b___ = orig[orig_pos___ + 0];

                const unsigned char a1 = orig[orig_pos + 3];
                const unsigned char a2 = orig[orig_pos_ + 3];
                const unsigned char a3 = orig[orig_pos__ + 3];
                const unsigned char a4 = orig[orig_pos___ + 3];

                const int pos = y * dest_pitch + x * dest_depth;

                dest[pos + 0] = (r * ax + r__ * ax_) * ay + (r___ * ax + r_ * ax_) * ay_;
                dest[pos + 1] = (g * ax + g__ * ax_) * ay + (g___ * ax + g_ * ax_) * ay_;
                dest[pos + 2] = (b * ax + b__ * ax_) * ay + (b___ * ax + b_ * ax_) * ay_;
                dest[pos + 3] = (a1 * ax + a3 * ax_) * ay + (a4 * ax + a2 * ax_) * ay_;
            }
        }
    image = texture;
}

GLWidget::vec2 GLWidget::project(const vec3 &point)
{
    vec3 ret = rotation * point;
    num z = std::max<num>(.75f, 1 + translation.z()/-60);
    int w = width(), h = height();
    num x = w * translation.x() / 2 / -40;
    if (std::abs(x) > w/2)
        x = x > 0 ? w/2 : w/-2;
    num y = h * translation.y() / 2 / -40;
    if (std::abs(y) > h/2)
        y = y > 0 ? h/2 : h/-2;
    return vec2(z * (ret.x() + x), z * (ret.y() + y));
}

GLWidget::vec3 GLWidget::project2(const vec3 &point)
{
    return rotation * point;
}

