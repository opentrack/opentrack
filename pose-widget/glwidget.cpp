/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "glwidget.h"
#include <cmath>
#include <algorithm>

#include <QPainter>
#include <QPaintEvent>

GLWidget::GLWidget(QWidget *parent) : QWidget(parent)
{
    Q_INIT_RESOURCE(posewidget);

    front = QImage(QString(":/images/side1.png"));
    back = QImage(QString(":/images/side6.png"));
    rotateBy(0, 0, 0, 0, 0, 0);
}

GLWidget::~GLWidget()
{
}

void GLWidget::paintEvent ( QPaintEvent * event ) {
    QPainter p(this);
    project_quad_texture();
    p.drawImage(event->rect(), texture);
}

void GLWidget::rotateBy(double xAngle, double yAngle, double zAngle, double x, double y, double z)
{

    double c1 = cos(yAngle / 57.295781);
    double s1 = sin(yAngle / 57.295781);
    double c2 = cos(xAngle / 57.295781);
    double s2 = sin(xAngle / 57.295781);
    double c3 = cos(zAngle / 57.295781);
    double s3 = sin(zAngle / 57.295781);

    double foo[] = {
        c2*c3,  -c2*s3,  s2,
        c1*s3+c3*s1*s2,  c1*c3-s1*s2*s3,  -c2*s1,
        s1*s3-c1*c3*s2,  c3*s1+c1*s2*s3,  c1*c2,
    };
    
    rotation = rmat(foo);
    translation = vec3({x, y, z});

    update();
}


static __inline double dot(const vec2& p1, const vec2& p2) {
    return p1(0,0) * p2(0,0) + p1(1,0) * p2(1,0);
}

class Triangle {
public:
    Triangle(const vec2& p1,
             const vec2& p2,
             const vec2& p3)
    {
        origin = p1;
        v0 = vec2({ p3(0,0) - p1(0,0), p3(1,0) - p1(1,0) });
        v1 = vec2({ p2(0,0) - p1(0,0), p2(1,0) - p1(1,0) });
        dot00 = dot(v0, v0);
        dot01 = dot(v0, v1);
        dot11 = dot(v1, v1);
        invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
    }
    bool barycentric_coords(const vec2& px, vec2& uv) const
    {
        vec2 v2({ px(0,0) - origin(0,0), px(1,0) - origin(1,0) });
        double dot12 = dot(v1, v2);
        double dot02 = dot(v0, v2);
        double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
        double v = (dot00 * dot12 - dot01 * dot02) * invDenom;
        uv = vec2({u, v});
        return (u >= 0) && (v >= 0) && (u + v <= 1);
    }
    
private:
    double dot00, dot01, dot11, invDenom;
    vec2 v0, v1, origin;
};

static __inline vec3 cross(const vec3& p1, const vec3& p2)
{
    return vec3({p1(1,0) * p2(2,0) - p2(1,0) * p1(2,0),
                 p2(0,0) * p1(2,0) - p1(0,0) * p2(2,0),
                 p1(0,0) * p2(1,0) - p1(1,0) * p2(0,0)});
}

static __inline vec3 normal(const vec3& p1, const vec3& p2, const vec3& p3)
{
    vec3 u({p2(0,0) - p1(0,0), p2(1,0) - p1(1,0), p2(2,0) - p1(2,0)});
    vec3 v({p3(0,0) - p1(0,0), p3(1,0) - p1(1,0), p3(2,0) - p1(2,0)});
     
    vec3 tmp = cross(u, v);
    
    double i = 1./sqrt(tmp(0,0) * tmp(0,0) + tmp(1,0) * tmp(1,0) + tmp(2,0) * tmp(2,0));
    
    return vec3({i * tmp(0,0), i * tmp(1,0), i * tmp(2,0)});
}

void GLWidget::project_quad_texture() {
    const int sx = width(), sy = height();
    vec2 pt[4];
    const vec3 corners[] = {
        vec3({0., 0., 0.}),
        vec3({sx-1., 0., 0.}),
        vec3({0., sy-1., 0.}),
        vec3({sx-1., sy-1., 0.})
    };
    
    for (int i = 0; i < 4; i++) {
        pt[i] = project(vec3({corners[i](0,0) - sx/2., corners[i](1,0) - sy/2., 0}));
        pt[i](0, 0) += sx/2.;
        pt[i](1, 0) += sy/2.;
    }
    
    vec3 normal1({0, 0, 1});
    vec3 normal2;
    {
        vec3 foo[3];
        for (int i = 0; i < 3; i++)
            foo[i] = project2(corners[i]);
        normal2 = normal(foo[0], foo[1], foo[2]);
    }
    
    double dir = normal1(0,0) * normal2(0,0) + normal1(1,0) * normal2(1,0) + normal1(2,0) * normal2(2,0);
    
    QImage& tex = dir < 0 ? back : front;
    
    int ow = tex.width(), oh = tex.height();

    vec2 p2[4];

    for (int i = 0; i < 4; i++)
        p2[i] = vec2({pt[i](0,0), pt[i](1,0)});
    QImage texture(QSize(sx, sy), QImage::Format_RGB888);
    QColor bgColor = palette().color(QPalette::Current, QPalette::Window);
    texture.fill(bgColor);
    
    const vec2 projected[2][3] = { { p2[0], p2[1], p2[2] }, { p2[3], p2[1], p2[2] } };
    const vec2 origs[2][3] = {
        { vec2({0., 0.}), vec2({ow-1., 0.}), vec2({0., oh-1.}) },
        { vec2({ow-1., oh-1.}), vec2({ow-1., 0.}), vec2({0., oh-1.}) }
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
            vec2 pos({(double)x, (double)y});
            for (int i = 0; i < 2; i++) {
                vec2 coords;
                if (triangles[i].barycentric_coords(pos, coords))
                {
                    int px = origs[i][0](0,0)
                            + coords(0,0) * (origs[i][2](0,0) - origs[i][0](0,0))
                            + coords(1,0) * (origs[i][1](0,0) - origs[i][0](0,0));
                    int py = origs[i][0](1,0)
                            + coords(0,0) * (origs[i][2](1,0) - origs[i][0](1,0))
                            + coords(1,0) * (origs[i][1](1,0) - origs[i][0](1,0));
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

vec2 GLWidget::project(const vec3 &point)
{
    vec3 ret = rotation * point;
    double z = std::max(.75, 1. + translation(2,0)/-60.);
    int w = width(), h = height();
    double x = w * translation(0, 0) / 2. / -40.;
    if (abs(x) > w/2)
        x = x > 0 ? w/2 : w/-2;
    double y = h * translation(1, 0) / 2. / -40.;
    if (abs(y) > h/2)
        y = y > 0 ? h/2 : h/-2;
    return vec2 { z * (ret(0, 0) + x), z * (ret(1, 0) + y) };
}

vec3 GLWidget::project2(const vec3 &point)
{
    return rotation * point;
}
