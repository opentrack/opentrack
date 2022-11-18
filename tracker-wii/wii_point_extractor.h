/*
* Copyright (c) 2017-2018 Wei Shuai <cpuwolf@gmail.com>
*
* Permission to use, copy, modify, and/or distribute this software for any
* purpose with or without fee is hereby granted, provided that the above
* copyright notice and this permission notice appear in all copies.
*/
#pragma once

#include "pt-api.hpp"

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace pt_module {

using namespace numeric_types;

class WIIPointExtractor final : public pt_point_extractor
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    void extract_points(const pt_frame& frame,
                        pt_preview& preview_frame,
                        bool preview_visible,
                        std::vector<vec2>& points) override;
    WIIPointExtractor(const QString& module_name);

private:
    pt_settings s;
    static void draw_point(cv::Mat& preview_frame, const vec2& p, const cv::Scalar& color, int thickness = 1);
    static void draw_points(cv::Mat& preview_frame, const struct wii_info& wii);
    static void draw_bg(cv::Mat& preview_frame, const struct wii_info& wii);
    static bool map_points(const struct wii_info& wii, std::vector<vec2>& points);
};

} // ns impl

