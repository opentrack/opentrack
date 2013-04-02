/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video_widget.h"

#include <QDebug>

using namespace cv;
using namespace std;

// ----------------------------------------------------------------------------
void VideoWidget::initializeGL()
{
	glClearColor(0.0, 0.0, 0.0, 0.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void VideoWidget::resizeGL(int w, int h)
{
	// setup 1 to 1 projection
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, w, 0, h, -1, 1);
	resize_frame();
    glDisable(GL_DEPTH_TEST);
    glBegin(GL_QUADS);
    glVertex2f(0,0);
    glVertex2f(1,0);
    glVertex2f(1,1);
    glVertex2f(0,1);
    glEnd();
}

void VideoWidget::paintGL()
{
    QMutexLocker lck(&mtx);
    if (resized_qframe.size() == size() || (resized_qframe.width() <= width() && resized_qframe.height() <= height())) {
        glDrawPixels(resized_qframe.width(), resized_qframe.height(), GL_RGB, GL_UNSIGNED_BYTE, resized_qframe.bits());
		
		const int crosshair_radius = 10;
		const int crosshair_thickness = 1;

		glColor3f(1.0, 0.0, 0.0);
		glLineWidth(crosshair_thickness);
		int x,y;
		for (vector<Vec2f>::iterator iter = points->begin();
			 iter != points->end();
			 ++iter)
		{
			x = (*iter)[0] * resized_qframe.width() + resized_qframe.width()/2.0  + 0.5;
			y = (*iter)[1] * resized_qframe.width() + resized_qframe.height()/2.0 + 0.5;
			
			glBegin(GL_LINES);
			glVertex2i(x-crosshair_radius, y);
			glVertex2i(x+crosshair_radius, y);
			glEnd();
			glBegin(GL_LINES);
			glVertex2i(x, y-crosshair_radius);
			glVertex2i(x, y+crosshair_radius);
			glEnd();
		}

    } else {
        glClear(GL_DEPTH_BUFFER_BIT);
    }
    glFlush();
}


void VideoWidget::resize_frame()
{
    QMutexLocker lck(&mtx);
#ifdef _WIN32
    if (qframe.size() == size() || (qframe.width() <= width() && qframe.height() <= height()))
        resized_qframe = qframe.mirrored();
    else
        resized_qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation).mirrored();
#else
    if (qframe.size() == size() || (qframe.width() <= width() && qframe.height() <= height()))
        resized_qframe = qframe.copy();
    else
        resized_qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
#endif
}

void VideoWidget::update()
{
    updateGL();
}

void VideoWidget::update_image(Mat frame, std::auto_ptr< vector<Vec2f> > points)
{
	this->frame = frame;
	this->points = points;

	// convert to QImage 
	if (frame.channels() == 3)
		qframe = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, frame.cols * 3, QImage::Format_RGB888).rgbSwapped();
	else if (frame.channels() == 1)
		qframe = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, frame.cols, QImage::Format_Indexed8);
	resize_frame();
}
