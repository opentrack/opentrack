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

void VideoWidget::update_image(Mat frame, std::auto_ptr< vector<Vec2f> > points)
{
    QMutexLocker((QMutex*)&mtx);
	this->frame = frame;
	this->points = points;
    
    QImage qframe;

	// convert to QImage 
	if (frame.channels() == 3)
		qframe = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, frame.cols * 3, QImage::Format_RGB888).rgbSwapped();
	else if (frame.channels() == 1)
		qframe = QImage((const unsigned char*)(frame.data), frame.cols, frame.rows, frame.cols, QImage::Format_Indexed8).convertToFormat(QImage::Format_RGB888);
    if (qframe.size() == size() || (qframe.width() <= width() && qframe.height() <= height()))
    {
        ;;;
    }
    else
        qframe = qframe.scaled(size(), Qt::IgnoreAspectRatio, Qt::FastTransformation);
    QPainter painter(&qframe);
    painter.setPen(Qt::red);
    painter.setBrush(Qt::red);
    if (points.get() != NULL) {
        const int crosshair_radius = 10;
        for (vector<Vec2f>::iterator iter = points->begin();
             iter != points->end();
             ++iter)
        {
            int x = (*iter)[0];
            int y = (*iter)[1];
            painter.drawLine(QLine(x-crosshair_radius, y, x+crosshair_radius, y));
            painter.drawLine(QLine(x, y-crosshair_radius, x, y+crosshair_radius));
        }
    }
    pixmap = QPixmap::fromImage(qframe);
}
