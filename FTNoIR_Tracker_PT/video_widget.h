/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include "frame_observer.h"

#include <QTime>
#include <QDialog>
#include <opencv2/opencv.hpp>
#ifndef OPENTRACK_API
#   include <QGLWidget>
#   include <boost/shared_ptr.hpp>
#else
#   include "FTNoIR_Tracker_PT/boost-compat.h"
#   if defined(_WIN32)
#       include <dshow.h>
#   endif
#endif
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>

#ifndef OPENTRACK_API
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
#else
/* Qt moc likes to skip over preprocessor directives -sh */
class VideoWidget2 : public QWidget, public FrameObserver
{
    Q_OBJECT

public:
    VideoWidget2(QWidget *parent, FrameProvider* provider) : QWidget(parent), /* to avoid linker errors */ FrameObserver(provider) {
        connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
        timer.start(45);
    }
    void update_image(const cv::Mat &frame);
    void update_frame_and_points() {}
protected slots:
    void paintEvent( QPaintEvent* e ) {
        QMutexLocker foo(&mtx);
        QPainter painter(this);
        painter.drawPixmap(e->rect(), pixmap, e->rect());
    }
private:
    QMutex mtx;
    QPixmap pixmap;
    QTimer timer;
};
#endif

// ----------------------------------------------------------------------------
// A VideoWidget embedded in a dialog frame
class VideoWidgetDialog : public QDialog
{
    Q_OBJECT
public:
	VideoWidgetDialog(QWidget *parent, FrameProvider* provider);
	virtual ~VideoWidgetDialog() {}

    VideoWidget2* get_video_widget() { return video_widget; }

private:
    VideoWidget2* video_widget;
};

#endif // VIDEOWIDGET_H
