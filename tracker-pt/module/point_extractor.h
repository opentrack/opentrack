/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
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

using namespace types;

struct blob final
{
    f radius, brightness;
    vec2 pos;
    cv::Rect rect;

    blob(f radius, const vec2& pos, f brightness, const cv::Rect& rect);
};

class PointExtractor final : public pt_point_extractor
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    void extract_points(const pt_frame& frame, pt_preview& preview_frame, std::vector<vec2>& points) override;
    PointExtractor(const QString& module_name);
private:
    static constexpr inline int max_blobs = 16;

    pt_settings s;

    cv::Mat1b frame_gray_unmasked, frame_bin, frame_gray;
    cv::Mat1f hist;
    std::vector<blob> blobs;
    cv::Mat1b ch[3];

    void ensure_channel_buffers(const cv::Mat& orig_frame);
    void ensure_buffers(const cv::Mat& frame);

    void extract_single_channel(const cv::Mat& orig_frame, int idx, cv::Mat& dest);

    void color_to_grayscale(const cv::Mat& frame, cv::Mat1b& output);
    void threshold_image(const cv::Mat& frame_gray, cv::Mat1b& output);
};

} // ns impl

