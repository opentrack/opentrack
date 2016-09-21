/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
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

#include <vector>

using namespace pt_types;

class PointExtractor final
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    // WARNING: returned reference is valid as long as object
    void extract_points(cv::Mat &frame, std::vector<vec2>& points);
    PointExtractor();

    settings_pt s;
private:
    static constexpr int max_blobs = 16;

    cv::Mat frame_gray;
    cv::Mat frame_bin;
    cv::Mat hist;
    cv::Mat frame_blobs;

    struct blob
    {
        double radius, brightness;
        vec2 pos;
        blob(double radius, const cv::Vec2d& pos, double brightness) : radius(radius), brightness(brightness), pos(pos)
        {
            //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1];
        }
    };

    std::vector<blob> blobs;
};

#endif //POINTEXTRACTOR_H
