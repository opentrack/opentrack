/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "ftnoir_tracker_pt_settings.h"
#include "camera.h"
#include "cv/numeric.hpp"

#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace pt_impl {

using namespace types;

struct blob
{
    f radius, brightness;
    vec2 pos;
    cv::Rect rect;

    blob(f radius, const vec2& pos, f brightness, cv::Rect &rect);
};

class PointExtractor final
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    void extract_points(const cv::Mat& frame, cv::Mat& preview_frame, std::vector<vec2>& points);
    PointExtractor();

    settings_pt s;

    static double threshold_radius_value(int w, int h, int threshold);
private:
    static constexpr int max_blobs = 16;

    cv::Mat1b frame_gray, frame_bin, frame_blobs;
    cv::Mat1f hist;
    std::vector<blob> blobs;
    cv::Mat1b ch[3];

    void ensure_channel_buffers(const cv::Mat& orig_frame);
    void ensure_buffers(const cv::Mat& frame);

    void extract_single_channel(const cv::Mat& orig_frame, int idx, cv::Mat& dest);
    void extract_channels(const cv::Mat& orig_frame, const int* order, int order_npairs);

    void color_to_grayscale(const cv::Mat& frame, cv::Mat1b& output);
    void threshold_image(const cv::Mat& frame_gray, cv::Mat1b& output);
};

} // ns impl

using pt_impl::PointExtractor;
