/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ar_video_widget.h"

#include <QDebug>

using namespace std;

void ArucoVideoWidget::update_image(const cv::Mat& frame)
{
    QMutexLocker foo(&mtx);
    _frame = frame.clone();
}

void ArucoVideoWidget::update_and_repaint()
{
    QMutexLocker foo(&mtx);
    if (_frame.cols*_frame.rows <= 0)
        return;
    QImage qframe = QImage(_frame.cols, _frame.rows, QImage::Format_RGB888);
    uchar* data = qframe.bits();
    const int pitch = qframe.bytesPerLine();
    for (int y = 0; y < _frame.rows; y++)
    {
        for (int x = 0; x < _frame.cols; x++)
        {
            const auto& elt = _frame.at<cv::Vec3b>(y, x);
            const cv::Scalar elt2 = static_cast<cv::Scalar>(elt);
            data[y * pitch + x * 3 + 0] = elt2.val[2];
            data[y * pitch + x * 3 + 1] = elt2.val[1];
            data[y * pitch + x * 3 + 2] = elt2.val[0];
        }
    }
    auto qframe2 = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    texture = qframe2;
    update();
}
