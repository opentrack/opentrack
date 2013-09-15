/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "frame_observer.h"

#include <QGLWidget>
#include <QTime>
#include <QDialog>
#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>

// ----------------------------------------------------------------------------
// OpenGL based widget to display an OpenCV image with some points on top
class VideoWidget : public QGLWidget, public FrameObserver
{
	Q_OBJECT

public:
	VideoWidget(QWidget *parent, FrameProvider* provider) : QGLWidget(parent), FrameObserver(provider) {}

	virtual void initializeGL();
	virtual void resizeGL(int w, int h);
	virtual void paintGL();

	void update_frame_and_points();

private:
	void resize_frame();

	cv::Mat frame;
	QImage qframe;
	QImage resized_qframe;

	boost::shared_ptr< std::vector<cv::Vec2f> > points;
};

// ----------------------------------------------------------------------------
// A VideoWidget embedded in a dialog frame
class VideoWidgetDialog : public QDialog
{
public:
	VideoWidgetDialog(QWidget *parent, FrameProvider* provider);
	virtual ~VideoWidgetDialog() {}

	VideoWidget* get_video_widget() { return video_widget; }

private:
	VideoWidget* video_widget;
};

#endif // VIDEOWIDGET_H
