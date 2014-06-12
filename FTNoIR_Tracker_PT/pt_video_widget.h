/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "frame_observer.h"
#include <QObject>
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

class PTVideoWidget : public QWidget, public FrameObserver
{
    Q_OBJECT

public:
    PTVideoWidget(QWidget *parent, FrameProvider* provider) :
        QWidget(parent),
        /* to avoid linker errors */ FrameObserver(provider),
        freshp(false)
    {
        connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()));
        timer.start(40);
    }
    void update_image(const cv::Mat &frame);
    void update_frame_and_points() {}
protected slots:
    void paintEvent( QPaintEvent* e ) {
        QMutexLocker foo(&mtx);
        QPainter painter(this);
        painter.drawImage(e->rect(), texture);
    }
    void update_and_repaint();
private:
    QMutex mtx;
    QImage texture;
    QTimer timer;
    cv::Mat _frame;
    bool freshp;
};

// ----------------------------------------------------------------------------
// A VideoWidget embedded in a dialog frame
class VideoWidgetDialog : public QDialog
{
    Q_OBJECT
public:
	VideoWidgetDialog(QWidget *parent, FrameProvider* provider);
	virtual ~VideoWidgetDialog() {}

    PTVideoWidget* get_video_widget() { return video_widget; }

private:
    PTVideoWidget* video_widget;
};
