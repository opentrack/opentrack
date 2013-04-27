/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "video_widget.h"

#include <QDebug>

using namespace cv;
using namespace std;

void VideoWidget::update_image(Mat frame, std::auto_ptr< vector<Vec2f> >)
{
    QMutexLocker foo(&mtx);

    if (frame.channels() != 3 && frame.channels() != 1)
        return;
    
    int width = frame.cols, height = frame.rows;
    unsigned char* src = frame.data;
    
    QImage qframe(width, height, QImage::Format_RGB888);
    if (frame.channels() == 3)
    {
        uchar* data = qframe.bits();
        const int pitch = qframe.bytesPerLine();
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
            {
                const int pos = 3 * (y*width + x);
                data[y * pitch + x * 3 + 0] = src[pos + 2];
                data[y * pitch + x * 3 + 1] = src[pos + 1];
                data[y * pitch + x * 3 + 2] = src[pos + 0];
            }
    } else {
        uchar* data = qframe.bits();
        const int pitch = qframe.bytesPerLine();
        for (int y = 0; y < height; y++)
            for (int x = 0; x < width; x++)
            {
                const int pos = (y*width + x);
                data[y * pitch + x * 3 + 0] = src[pos];
                data[y * pitch + x * 3 + 1] = src[pos];
                data[y * pitch + x * 3 + 2] = src[pos];
            }
    }
    qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    pixmap = QPixmap::fromImage(qframe);
}
