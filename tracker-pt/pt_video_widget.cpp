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

void PTVideoWidget::update_image(const cv::Mat& frame)
{
    QMutexLocker foo(&mtx);
    
    if (!freshp)
    {
        if (_frame.cols != frame.cols ||
            _frame.rows != frame.rows ||
            _frame.channels() != frame.channels())
        {
            _frame = cv::Mat();
        }
        frame.copyTo(_frame);
        freshp = true;
    }
}

void PTVideoWidget::update_and_repaint()
{
    {
        QMutexLocker foo(&mtx);
        if (_frame.empty() || !freshp)
            return;
        texture = QImage(_frame.cols, _frame.rows, QImage::Format_RGB888);
        freshp = false;
        uchar* data = texture.bits();
        const int chans = _frame.channels();
        const int pitch = texture.bytesPerLine();
        for (int y = 0; y < _frame.rows; y++)
        {
            unsigned char* dest = data + pitch * y;
            const unsigned char* ln = _frame.ptr(y);
            for (int x = 0; x < _frame.cols; x++)
            {
                const int idx = x * chans;
                const int x_ = x * 3;
                dest[x_ + 0] = ln[idx + 2];
                dest[x_ + 1] = ln[idx + 1];
                dest[x_ + 2] = ln[idx + 0];
            }
        }
    }
    texture = texture.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    update();
}
