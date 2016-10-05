/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video-widget.hpp"
#include <opencv2/imgproc.hpp>

cv_video_widget::cv_video_widget(QWidget* parent) :
    QWidget(parent),
    freshp(false)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()));
    timer.start(50);
}

void cv_video_widget::update_image(const cv::Mat& frame)
{
    QMutexLocker foo(&mtx);

    if (!freshp)
    {
        if (_frame.cols != frame.cols || _frame.rows != frame.rows)
            _frame = cv::Mat(frame.rows, frame.cols, CV_8UC3);
        frame.copyTo(_frame);
        freshp = true;
    }
}

void cv_video_widget::paintEvent(QPaintEvent*)
{
    QMutexLocker foo(&mtx);
    QPainter painter(this);
    painter.drawImage(rect(), texture);
}

void cv_video_widget::update_and_repaint()
{
    QMutexLocker l(&mtx);

    if (_frame.empty() || !freshp)
        return;

    if (_frame2.cols != _frame.cols || _frame2.rows != _frame.rows)
        _frame2 = cv::Mat(_frame.rows, _frame.cols, CV_8UC3);

    if (_frame3.cols != width() || _frame3.rows != height())
        _frame3 = cv::Mat(height(), width(), CV_8UC3);

    cv::cvtColor(_frame, _frame2, cv::COLOR_RGB2BGR);
    cv::resize(_frame2, _frame3, cv::Size(width(), height()), 0, 0, cv::INTER_NEAREST);

    texture = QImage((const unsigned char*) _frame3.data, _frame3.cols, _frame3.rows, QImage::Format_RGB888);
    freshp = false;
    update();
}
