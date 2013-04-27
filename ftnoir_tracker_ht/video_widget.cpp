/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video_widget.h"

#include <QDebug>

using namespace std;

void VideoWidget::update_image(unsigned char *frame, int width, int height)
{
    QMutexLocker((QMutex*)&mtx);
    QImage qframe = QImage(frame, width, height, 3 * width, QImage::Format_RGB888).rgbSwapped().mirrored();
    if (qframe.size() == size() || (qframe.width() <= this->width() && qframe.height() <= this->height()))
        qframe = qframe.mirrored();
    else
        qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation).mirrored();
    QPainter painter(&qframe);
    painter.setPen(Qt::blue);
    painter.setBrush(Qt::blue);
    pixmap = QPixmap::fromImage(qframe);
}
