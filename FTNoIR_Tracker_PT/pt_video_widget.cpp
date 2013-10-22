/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * 20130312, WVR: Add 7 lines to resizeGL after resize_frame. This should lower CPU-load.
 */

#include "pt_video_widget.h"

#include <QDebug>
#include <QHBoxLayout>

using namespace cv;
using namespace std;

void PTVideoWidget::update_image(const cv::Mat& frame)
{
    QMutexLocker foo(&mtx);
    _frame = frame;
}

// ----------------------------------------------------------------------------
VideoWidgetDialog::VideoWidgetDialog(QWidget *parent, FrameProvider* provider)
    : QDialog(parent),
      video_widget(NULL)
{
    const int VIDEO_FRAME_WIDTH  = 640;
    const int VIDEO_FRAME_HEIGHT = 480;

    video_widget = new PTVideoWidget(this, provider);

    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(video_widget);
    if (this->layout()) delete this->layout();
    setLayout(layout);
    resize(VIDEO_FRAME_WIDTH, VIDEO_FRAME_HEIGHT);
}

void PTVideoWidget::update_and_repaint()
{
    QMutexLocker foo(&mtx);
    if (_frame.empty())
        return;
    QImage qframe = QImage(_frame.cols, _frame.rows, QImage::Format_RGB888);
    uchar* data = qframe.bits();
    const int pitch = qframe.bytesPerLine();
    for (int y = 0; y < _frame.rows; y++)
        for (int x = 0; x < _frame.cols; x++)
        {
            const int pos = 3 * (y*_frame.cols + x);
            data[y * pitch + x * 3 + 0] = _frame.data[pos + 2];
            data[y * pitch + x * 3 + 1] = _frame.data[pos + 1];
            data[y * pitch + x * 3 + 2] = _frame.data[pos + 0];
        }
    qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    pixmap = QPixmap::fromImage(qframe);
    update();
}
