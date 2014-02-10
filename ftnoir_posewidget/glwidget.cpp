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
    p.drawImage(event->rect(), texture);
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

class Triangle {
public:
    Triangle(const Vec2f& p1,
             const Vec2f& p2,
             const Vec2f& p3)
    {
        origin = p1;
        v0 = Vec2f(p3.x - p1.x, p3.y - p1.y);
        v1 = Vec2f(p2.x - p1.x, p2.y - p1.y);
        dot00 = dot(v0, v0);
        dot01 = dot(v0, v1);
        dot11 = dot(v1, v1);
        invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    }
    bool barycentric_coords(const Vec2f& px, Vec2f& uv) const
    {
        Vec2f v2(px.x - origin.x, px.y - origin.y);
        double dot12 = dot(v1, v2);
        double dot02 = dot(v0, v2);
        double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        uv.x = u;
        uv.y = v;
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }
    
private:
    double dot00, dot01, dot11, invDenom;
    Vec2f v0, v1, origin;
    double dot(const Vec2f& p1, const Vec2f& p2) const {
        return p1.x * p2.x + p1.y * p2.y;
    }
};

static __inline Vec3f cross(const Vec3f& p1, const Vec3f& p2)
{
    return Vec3f(p1.y * p2.z - p2.y * p1.z,
                 p2.x * p1.z - p1.x * p2.z,
                 p1.x * p2.y - p1.y * p2.x);
}

static __inline Vec3f normal(const Vec3f& p1, const Vec3f& p2, const Vec3f& p3)
{
    Vec3f u(p2.x - p1.x, p2.y - p1.y, p2.z - p1.z);
    Vec3f v(p3.x - p1.x, p3.y - p1.y, p3.z - p1.z);
     
    Vec3f tmp = cross(u, v);
    
    double i = 1./sqrt(tmp.x * tmp.x + tmp.y * tmp.y + tmp.z * tmp.z);
    
    return Vec3f(i * tmp.x, i * tmp.y, i * tmp.z);
}

void GLWidget::project_quad_texture() {
    const int sx = width(), sy = height();
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
    QColor bgColor = palette().color(QPalette::Current, QPalette::Window);
    texture.fill(bgColor);
    
    const Vec2f projected[2][3] = { { p2[0], p2[1], p2[2] }, { p2[3], p2[1], p2[2] } };
    const Vec2f origs[2][3] = {
        { Vec2f(0, 0), Vec2f(ow-1, 0), Vec2f(0, oh-1) },
        { Vec2f(ow-1, oh-1), Vec2f(ow-1, 0), Vec2f(0, oh-1) }
    };
    const Triangle triangles[2] = {
        Triangle(projected[0][0], projected[0][1], projected[0][2]),
        Triangle(projected[1][0], projected[1][1], projected[1][2])
    };
  
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
                if (triangles[i].barycentric_coords(pos, coords))
                {
                    int px = origs[i][0].x
                            + coords.x * (origs[i][2].x - origs[i][0].x)
                            + coords.y * (origs[i][1].x - origs[i][0].x);
                    int py = origs[i][0].y
                            + coords.x * (origs[i][2].y - origs[i][0].y)
                            + coords.y * (origs[i][1].y - origs[i][0].y);
                    int r = orig[py * orig_pitch + px * orig_depth + 2];
                    int g = orig[py * orig_pitch + px * orig_depth + 1];
                    int b = orig[py * orig_pitch + px * orig_depth + 0];

                    dest[y * dest_pitch + x * dest_depth + 0] = r;
                    dest[y * dest_pitch + x * dest_depth + 1] = g;
                    dest[y * dest_pitch + x * dest_depth + 2] = b;

                    break;
                }
            }
        }
    this->texture = texture;
}
