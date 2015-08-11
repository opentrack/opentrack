/* Copyright (c) 2013, 2015 Stanislaw Halik <sthalik@misaki.pl>
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

typedef Mat<float, 2, 1> vec2;
typedef Mat<float, 3, 1> vec3;
typedef Mat<float, 3, 3> rmat;

class GLWidget : public QWidget
{
public:
    using num = float;
    GLWidget(QWidget *parent);
    ~GLWidget();
    void rotateBy(float xAngle, float yAngle, float zAngle, float x, float y, float z);
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
