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
    QImage qframe = QImage(width, height, QImage::Format_RGB888);
    uchar* data = qframe.bits();
    const int pitch = qframe.bytesPerLine();
    for (int y = 0; y < height; y++)
        for (int x = 0; x < width; x++)
        {
            const int pos = 3 * (y*width + x);
            data[y * pitch + x * 3 + 0] = frame[pos + 2];
            data[y * pitch + x * 3 + 1] = frame[pos + 1];
            data[y * pitch + x * 3 + 2] = frame[pos + 0];
        }
    qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    pixmap = QPixmap::fromImage(qframe);
}
