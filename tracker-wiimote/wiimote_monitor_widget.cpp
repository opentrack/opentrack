/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2016 fred41
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
 
#include "wiimote_monitor_widget.h"

void WiiMoteMonitorWidget::update_image(const std::vector<cv::Vec2f>& p)
{

    if (image_.isNull()) {
        frame = cv::Mat( height() - 1, width(), CV_8U, cv::Scalar(0));
        image_ = QImage((const unsigned char*) frame.data, width(), height() - 1, QImage::Format_Grayscale8);
    }

    frame = cv::Scalar(0);

    for (unsigned i = 0; i < p.size(); i++)
    {
        cv::Point p_ = cv::Point(-p[i][0] * frame.cols + frame.cols * 0.5f, p[i][1] * frame.cols + frame.rows * 0.5f);
        cv::circle(frame, p_, 3, cv::Scalar(255), 1);
    }        

    update();
}
