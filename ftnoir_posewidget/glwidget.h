/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef GLWIDGET_H
#define GLWIDGET_H

#include <QtGui>
#include <QPixmap>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"

struct Point {
    Point(int x, int y) :
            x(x), y(y)
    {
    }
    Point() :
            x(0), y(0)
    {
    }
    int x, y;
};

struct Vec3f {
    double x, y, z;
    Vec3f(double x, double y, double z) :
            x(x), y(y), z(z)
    {
    }
    Vec3f() :
            x(0), y(0), z(0)
    {
    }
};

struct Vec2f {
    double x, y;
    Vec2f(double x, double y) :
            x(x), y(y)
    {
    }
    Vec2f() :
            x(0), y(0)
    {
    }
};

class FTNOIR_TRACKER_BASE_EXPORT GLWidget : public QWidget
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent);
    ~GLWidget();
    void rotateBy(double xAngle, double yAngle, double zAngle);
    
protected:
    void paintEvent ( QPaintEvent * event );

private:
    Point project(const Vec3f& point) {
        Point rect;

        rect.x = point.x * matrix[0]
                 + point.y * matrix[1]
                 + point.z * matrix[2];
        rect.y = point.x * matrix[3]
                 + point.y * matrix[4]
                 + point.z * matrix[5];

        return rect;
    }
    Vec3f project2(const Vec3f& point) {
        Vec3f rect;

        rect.x = point.x * matrix[0]
                 + point.y * matrix[1]
                 + point.z * matrix[2];
        rect.y = point.x * matrix[3]
                 + point.y * matrix[4]
                 + point.z * matrix[5];
        rect.z = point.x * matrix[6]
                 + point.y * matrix[7]
                 + point.z * matrix[8];
        return rect;
    }
    void project_quad_texture();
    double matrix[9];
    QImage front;
    QImage back;
    QImage texture;
};

#endif
