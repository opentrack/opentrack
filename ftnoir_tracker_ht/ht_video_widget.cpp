/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ht_video_widget.h"

#include <QDebug>

using namespace std;

void HTVideoWidget::update_image(unsigned char *frame, int width, int height)
{
    QMutexLocker foo(&mtx);
    memcpy(fb, frame, width * height * 3);
    this->width = width;
    this->height = height;
}

void HTVideoWidget::update_and_repaint()
{
    QMutexLocker foo(&mtx);
    if (width*height <= 0)
        return;
    QImage qframe = QImage(width, height, QImage::Format_RGB888);
    uchar* data = qframe.bits();
    const int pitch = qframe.bytesPerLine();
    for (int y = 0; y < height; y++)
    {
        const int part = y*width;
        for (int x = 0; x < width; x++)
        {
            const int pos = 3 * (part + x);
            data[y * pitch + x * 3 + 0] = fb[pos + 2];
            data[y * pitch + x * 3 + 1] = fb[pos + 1];
            data[y * pitch + x * 3 + 2] = fb[pos + 0];
        }
    }
    auto qframe2 = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    texture = qframe2;
    update();
}
