/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include <QtGui>
#include "glwidget.h"
#include <QWidget>
#include <cmath>
#include <algorithm>

GLWidget::GLWidget(QWidget *parent) : QWidget(parent)
{
    front = QImage(QString(":/images/side1.png"));
    back = QImage(QString(":/images/side6.png"));
    rotateBy(0, 0, 0);
}

GLWidget::~GLWidget()
{
}

void GLWidget::paintEvent ( QPaintEvent * event ) {
    QWidget::paintEvent(event);
    QPainter p(this);
    project_quad_texture();
    p.drawPixmap(event->rect(), pixmap);
}

void GLWidget::rotateBy(double xAngle, double yAngle, double zAngle)
{
    
    double ch = cos(xAngle / 57.295781);
    double sh = sin(xAngle / 57.295781);
    double ca = cos(yAngle / 57.295781);
    double sa = sin(yAngle / 57.295781);
    double cb = cos(zAngle / 57.295781);
    double sb = sin(zAngle / 57.295781);

    matrix[0 * 3 + 0] = ch * ca;
    matrix[0 * 3 + 1]= sh*sb - ch*sa*cb;
    matrix[0 * 3 + 2]= ch*sa*sb + sh*cb;
    matrix[1 * 3 + 0]= sa;
    matrix[1 * 3 + 1]= ca*cb;
    matrix[1 * 3 + 2]= -ca*sb;
    matrix[2 * 3 + 0]= -sh*ca;
    matrix[2 * 3 + 1]= sh*sa*cb + ch*sb;
    matrix[2 * 3 + 2]= -sh*sa*sb + ch*cb;
    update();
}

static __inline double dot(const Vec2f& p1, const Vec2f& p2) {
    return p1.x * p2.x + p1.y * p2.y;
}

static bool barycentric_coords(const Vec2f& p1,
                               const Vec2f& p2,
                               const Vec2f& p3,
                               const Vec2f& px,
                               Vec2f& uv)
{
    Vec2f v0(p3.x - p1.x, p3.y - p1.y);
    Vec2f v1(p2.x - p1.x, p2.y - p1.y);
    Vec2f v2(px.x - p1.x, px.y - p1.y);

    double dot00 = dot(v0, v0);
    double dot01 = dot(v0, v1);
    double dot02 = dot(v0, v2);
    double dot11 = dot(v1, v1);
    double dot12 = dot(v1, v2);

    double invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    uv.x = u;
    uv.y = v;

    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

static __inline Vec3f normal(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3)
{
    Vec3f u(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    Vec3f v(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
     
    Vec3f tmp(u.y * v.z - u.z * v.y,
              u.z * v.x - u.x * v.z,
              u.x * v.y - u.y * v.x);
    
    double i = 1./sqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
    
    return Vec3f(i * tmp.x, i * tmp.y, i * tmp.z);
}

static __inline Vec3f cross(const Vec3f& p1, const Vec3f& p2)
{
    return Vec3f(p1.y * p2.z - p2.y * p1.z,
                 p2.x * p1.z - p1.x * p2.z,
                 p1.x * p2.y - p1.y * p2.x);
}

void GLWidget::project_quad_texture() {
    const int sx = 90, sy = 90;
    Point pt[4];
    static Vec3f corners[] = {
        Vec3f(0, 0, 0),
        Vec3f(sx-1, 0, 0),
        Vec3f(0, sy-1, 0),
        Vec3f(sx-1, sy-1, 0)
    };
    
    for (int i = 0; i < 4; i++) {
        pt[i] = project(Vec3f(corners[i].x - sx/2, corners[i].y - sy/2, 0));
        pt[i].x += sx/2;
        pt[i].y += sy/2;
    }
    
    Vec3f normal1(0, 0, 1);
    Vec3f normal2;
    {
        Vec3f foo[3];
        for (int i = 0; i < 3; i++)
            foo[i] = project2(corners[i]);
        normal2 = normal(foo[0], foo[1], foo[2]);
    }
    
    double dir = normal1.x * normal2.x + normal1.y * normal2.y + normal1.z * normal2.z;
    
    QImage& tex = dir < 0 ? back : front;
    
    int ow = tex.width(), oh = tex.height();
       
    Vec2f p2[4];

    for (int i = 0; i < 4; i++)
        p2[i] = Vec2f(pt[i].x, pt[i].y);
    
    QImage texture(QSize(sx, sy), QImage::Format_RGB888);
    texture.fill(Qt::black);
    
    const Vec2f projected[2][3] = { { p2[0], p2[1], p2[2] }, { p2[3], p2[1], p2[2] } };
    const Vec2f origs[2][3] = {
        { Vec2f(0, 0), Vec2f(ow-1, 0), Vec2f(0, oh-1) },
        { Vec2f(ow-1, oh-1), Vec2f(ow-1, 0), Vec2f(0, oh-1) }
    };
  
    double sqrt2 = std::sqrt(2.0);
    int orig_pitch = tex.bytesPerLine();
    int dest_pitch = texture.bytesPerLine();
    
    const unsigned char* orig = tex.bits();
    unsigned char* dest = texture.bits();
    
    int orig_depth = tex.depth() / 8;
    int dest_depth = texture.depth() / 8;
    
    /* image breakage? */
    if (orig_depth < 3)
        return;

    for (int y = 0; y < sy; y++)
        for (int x = 0; x < sx; x++) {
            Vec2f pos;
            pos.x = x;
            pos.y = y;
            for (int i = 0; i < 2; i++) {
                Vec2f coords;
                if (barycentric_coords(projected[i][0],
                                       projected[i][1],
                                       projected[i][2],
                                       pos,
                                       coords))
                {
                    double qx = origs[i][0].x
                                + coords.x * (origs[i][2].x - origs[i][0].x)
                                + coords.y * (origs[i][1].x - origs[i][0].x);
                    double qy = origs[i][0].y
                                + coords.x * (origs[i][2].y - origs[i][0].y)
                                + coords.y * (origs[i][1].y - origs[i][0].y);
                    int qx1 = std::min<int>(ow - 1, std::max<int>(0, qx - 0.5));
                    int qy1 = std::min<int>(oh - 1, std::max<int>(0, qy - 0.5));
                    int qx2 = std::min<int>(ow - 1, std::max<int>(0, qx + 0.5));
                    int qy2 = std::min<int>(oh - 1, std::max<int>(0, qy + 0.5));

                    double dx1 = qx1 - qx;
                    double dy1 = qy1 - qy;
                    double dx2 = qx2 - qx;
                    double dy2 = qy2 - qy;

                    double d1 = sqrt2 - std::sqrt(dx1 * dx1 + dy1 * dy1);
                    double d2 = sqrt2 - std::sqrt(dx2 * dx2 + dy2 * dy2);
                    double d3 = sqrt2 - std::sqrt(dx2 * dx2 + dy1 * dy1);
                    double d4 = sqrt2 - std::sqrt(dx1 * dx1 + dy2 * dy2);

                    double inv_norm = 1. / (d1 + d2 + d3 + d4);

                    d1 *= inv_norm;
                    d2 *= inv_norm;
                    d3 *= inv_norm;
                    d4 *= inv_norm;
                    
                    double r = d1 * (double) orig[qy1 * orig_pitch + qx1 * orig_depth + 2]
                               + d2 * (double) orig[qy2 * orig_pitch + qx2 * orig_depth + 2]
                               + d3 * (double) orig[qy1 * orig_pitch + qx2 * orig_depth + 2]
                               + d4 * (double) orig[qy2 * orig_pitch + qx1 * orig_depth + 2];
                    
                    double g = d1 * (double) orig[qy1 * orig_pitch + qx1 * orig_depth + 1]
                               + d2 * (double) orig[qy2 * orig_pitch + qx2 * orig_depth + 1]
                               + d3 * (double) orig[qy1 * orig_pitch + qx2 * orig_depth + 1]
                               + d4 * (double) orig[qy2 * orig_pitch + qx1 * orig_depth + 1];
                    
                    double b = d1 * (double) orig[qy1 * orig_pitch + qx1 * orig_depth + 0]
                               + d2 * (double) orig[qy2 * orig_pitch + qx2 * orig_depth + 0]
                               + d3 * (double) orig[qy1 * orig_pitch + qx2 * orig_depth + 0]
                               + d4 * (double) orig[qy2 * orig_pitch + qx1 * orig_depth + 0];
                    
                    dest[y * dest_pitch + x * dest_depth + 0] = std::max<int>(0, std::min<int>(255, r));
                    dest[y * dest_pitch + x * dest_depth + 1] = std::max<int>(0, std::min<int>(255, g));
                    dest[y * dest_pitch + x * dest_depth + 2] = std::max<int>(0, std::min<int>(255, b));

                    break;
                }
            }
}
    pixmap = QPixmap::fromImage(texture);
}