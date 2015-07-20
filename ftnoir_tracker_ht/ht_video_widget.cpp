/* Copyright (c) 2014 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ht_video_widget.h"

using namespace std;

void HTVideoWidget::update_image(unsigned char *frame, int width, int height)
{
    QMutexLocker foo(&mtx);
    if (!fresh)
    {
        memcpy(fb, frame, width * height * 3);
        this->width = width;
        this->height = height;
        fresh = true;
    }
}

void HTVideoWidget::update_and_repaint()
{
    QImage qframe;
    {
        QMutexLocker foo(&mtx);
        if (width*height <= 0 || !fresh)
            return;
        fresh = false;
        qframe = QImage(width, height, QImage::Format_RGB888);
        uchar* data = qframe.bits();
        const int pitch = qframe.bytesPerLine();
        for (int y = 0; y < height; y++)
        {
            const int part = y*width;
            for (int x = 0; x < width; x++)
            {
                const int pos = 3 * (part + x);
                const int x_ = x * 3;
                data[x_ + 0] = fb[pos + 2];
                data[x_ + 1] = fb[pos + 1];
                data[x_ + 2] = fb[pos + 0];
            }
            data += pitch;
        }
    }
    qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    {
        QMutexLocker foo(&mtx);
        texture = qframe;
    }
    update();
}
