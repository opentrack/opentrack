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

    cv::Mat frame_gray;
    cv::Mat frame_bin;
    cv::Mat hist;
    cv::Mat frame_blobs;
    std::vector<blob> blobs;
    cv::Mat ch[3], ch_float[4];

    void ensure_channel_buffers(const cv::Mat& orig_frame);
    void ensure_buffers(const cv::Mat& frame);

    void extract_single_channel(const cv::Mat& orig_frame, int idx, cv::Mat& dest);
    void extract_channels(const cv::Mat& orig_frame, const int* order, int order_npairs);
    void extract_all_channels(const cv::Mat& orig_frame);
    void channels_to_float(unsigned num_channels);

    void color_to_grayscale(const cv::Mat& frame, cv::Mat& output);
    void threshold_image(const cv::Mat& frame_gray, cv::Mat& output);
};

} // ns impl

using pt_impl::PointExtractor;
