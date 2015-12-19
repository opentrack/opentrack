/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * 20130312, WVR: Add 7 lines to resizeGL after resize_frame. This should lower CPU-load.
 */

#include "pt_video_widget.h"
#include <opencv2/imgproc.hpp>

void PTVideoWidget::update_image(const cv::Mat& frame)
{
    QMutexLocker foo(&mtx);
    
    if (!freshp)
    {
        if (_frame.cols != frame.cols || _frame.rows != frame.rows)
        {
            _frame = cv::Mat(frame.rows, frame.cols, CV_8U);
            _frame2 = cv::Mat(frame.rows, frame.cols, CV_8U);
        }
        frame.copyTo(_frame);
        freshp = true;
    }
}

void PTVideoWidget::update_and_repaint()
{
    QMutexLocker foo(&mtx);
    if (_frame.empty() || !freshp)
        return;
    cv::cvtColor(_frame, _frame2, cv::COLOR_RGB2BGR);
    
    if (_frame3.cols != width() || _frame3.rows != height())
        _frame3 = cv::Mat(height(), width(), CV_8U);
    
    cv::resize(_frame2, _frame3, cv::Size(width(), height()), 0, 0, cv::INTER_NEAREST);
    
    texture = QImage((const unsigned char*) _frame2.data, _frame2.cols, _frame2.rows, QImage::Format_RGB888);
    freshp = false;
    update();
}
