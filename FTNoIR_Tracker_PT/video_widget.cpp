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
using namespace boost;

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
}

void VideoWidget::paintGL()
{
	glClear(GL_COLOR_BUFFER_BIT);
	if (!resized_qframe.isNull())
	{
		glDrawPixels(resized_qframe.width(), resized_qframe.height(), GL_RGBA, GL_UNSIGNED_BYTE, resized_qframe.bits());
		
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
	}
	glFlush();
}


void VideoWidget::resize_frame()
{
	if (!qframe.isNull())
		resized_qframe = qframe.scaled(this->size(), Qt::KeepAspectRatio);
}


void VideoWidget::update(Mat frame, shared_ptr< vector<Vec2f> > points)
{
	this->frame = frame;
	this->points = points;

	// convert to QImage
	if (frame.channels() == 3)
		qframe = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, frame.step, QImage::Format_RGB888).rgbSwapped();
	else if (frame.channels() == 1)
		qframe = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, frame.step, QImage::Format_Indexed8);
	qframe = QGLWidget::convertToGLFormat(qframe);

	resize_frame();
	updateGL();
}
