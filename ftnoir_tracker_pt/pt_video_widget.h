/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <QObject>
#include <QTime>
#include <QDialog>
#include <opencv2/core/core.hpp>
#ifndef OPENTRACK_API
#   include <QGLWidget>
#   include <boost/shared_ptr.hpp>
#else
#   include <memory>
#   if defined(_WIN32)
#       include <dshow.h>
#   endif
#endif
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <QMutex>
#include <QMutexLocker>

class PTVideoWidget : public QWidget
{
    Q_OBJECT

public:
    PTVideoWidget(QWidget *parent) :
        QWidget(parent),
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
    volatile bool freshp;
};
