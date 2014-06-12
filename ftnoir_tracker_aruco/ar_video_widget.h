/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef VIDEOWIDGET_H
#define VIDEOWIDGET_H

#include <QTimer>
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QTimer>
#include <opencv/cv.hpp>

// ----------------------------------------------------------------------------
class ArucoVideoWidget : public QWidget
{
	Q_OBJECT

public:
    ArucoVideoWidget(QWidget *parent) : QWidget(parent) {
        connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()));
        timer.start(60);
	}
    void update_image(const cv::Mat& frame);
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
};

#endif // VIDEOWIDGET_H
