/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QGLWidget>
#include <QTime>
#include <opencv2/opencv.hpp>
#include <boost/shared_ptr.hpp>

// ----------------------------------------------------------------------------
class VideoWidget : public QGLWidget
{
	Q_OBJECT

public:
	VideoWidget(QWidget *parent) : QGLWidget(parent) {}

	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();

	void update(cv::Mat frame, boost::shared_ptr< std::vector<cv::Vec2f> > points);

private:
	void resize_frame();

	cv::Mat frame;
	QImage qframe;
	QImage resized_qframe;

	boost::shared_ptr< std::vector<cv::Vec2f> > points;
};

#endif // VIDEOWIDGET_H
