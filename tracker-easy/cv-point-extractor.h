/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "tracker-easy-api.h"

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>


using namespace numeric_types;


class CvPointExtractor final : public pt_point_extractor
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    void extract_points(const cv::Mat& frame, cv::Mat& preview_frame, std::vector<vec2>& points, std::vector<vec2>& imagePoints) override;
    CvPointExtractor(const QString& module_name);

    pt_settings s;
};



