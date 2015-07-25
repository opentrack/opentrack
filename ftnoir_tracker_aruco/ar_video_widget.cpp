/* Copyright (c) 2014 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ar_video_widget.h"

void ArucoVideoWidget::update_image(const cv::Mat& frame)
{
    QMutexLocker foo(&mtx);
    if (!fresh)
    {
        _frame = frame.clone();
        fresh = true;
    }
}

void ArucoVideoWidget::update_and_repaint()
{
    QImage qframe;
    
    {
        QMutexLocker foo(&mtx);
        if (_frame.cols*_frame.rows <= 0 || !fresh)
            return;
        fresh = false;
        qframe = QImage(_frame.cols, _frame.rows, QImage::Format_RGB888);
        uchar* data = qframe.bits();
        const int pitch = qframe.bytesPerLine();
        unsigned char *input = (unsigned char*)(_frame.data);
        const int chans = _frame.channels();
        for (int y = 0; y < _frame.rows; y++)
        {
            const int step = y * _frame.step;
            const int pitch_ = y * pitch;
            for (int x = 0; x < _frame.cols; x++)
            {
                data[pitch_ + x * 3 + 0] = input[step + x * chans + 2];
                data[pitch_ + x * 3 + 1] = input[step + x * chans + 1];
                data[pitch_ + x * 3 + 2] = input[step + x * chans + 0];
            }
        }
    }
    
    qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    
    {
        QMutexLocker foo(&mtx);
        texture = qframe;
    }
    
    update();
}

void ArucoVideoWidget::paintEvent(QPaintEvent* e)
{
    QMutexLocker foo(&mtx);
    QPainter(this).drawImage(e->rect(), texture);
}

ArucoVideoWidget::ArucoVideoWidget(QWidget* parent): QWidget(parent), fresh(false)
{
    connect(&timer, SIGNAL(timeout()), this, SLOT(update_and_repaint()));
    timer.start(60);
}
