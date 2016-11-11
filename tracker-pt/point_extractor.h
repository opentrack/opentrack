/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ftnoir_tracker_pt_settings.h"
#include "numeric.hpp"

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

#include <vector>

namespace impl {

using namespace types;

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

} // ns impl

using impl::PointExtractor;
