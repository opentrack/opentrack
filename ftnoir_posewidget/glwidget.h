/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* Adopted this widget from the 'textures' sample of the Nokia Qt toolkit.		*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*********************************************************************************/

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
    QPixmap pixmap;
};

#endif
