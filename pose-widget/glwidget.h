/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QWidget>
#include <QPixmap>
#include "opentrack/plugin-api.hpp"
#include "opentrack/simple-mat.hpp"

typedef dmat<2, 1> vec2;
typedef dmat<3, 1> vec3;
typedef dmat<3, 3> rmat;

class GLWidget : public QWidget
{
public:
    GLWidget(QWidget *parent);
    ~GLWidget();
    void rotateBy(double xAngle, double yAngle, double zAngle, double x, double y, double z);
protected:
    void paintEvent ( QPaintEvent * event ) override;
private:
    vec2 project(const vec3& point);
    vec3 project2(const vec3& point);
    void project_quad_texture();
    rmat rotation;
    vec3 translation;
    QImage front;
    QImage back;
    QImage texture;
};
