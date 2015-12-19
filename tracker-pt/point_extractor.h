/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTEXTRACTOR_H
#define POINTEXTRACTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ftnoir_tracker_pt_settings.h"

#include <QMutex>

class PointExtractor
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    // WARNING: returned reference is valid as long as object
    const std::vector<cv::Vec2f> &extract_points(cv::Mat &frame);
    const std::vector<cv::Vec2f>& get_points() { QMutexLocker l(&mtx); return points; }
    PointExtractor();
    
    settings_pt s;
private:
    std::vector<cv::Vec2f> points;
    QMutex mtx;
};

#endif //POINTEXTRACTOR_H
