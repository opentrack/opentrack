/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
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

using namespace types;

class WIIPointExtractor final : public pt_point_extractor
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    void extract_points(const pt_frame& frame, pt_preview& preview_frame, std::vector<vec2>& points) override;
    WIIPointExtractor(const QString& module_name);
private:
    static constexpr int max_blobs = 16;

    pt_settings s;
	void _draw_point(cv::Mat& preview_frame, const vec2& p, const cv::Scalar& color, int thinkness = 1);
	bool _draw_points(cv::Mat& preview_frame, const struct wii_info &wii, std::vector<vec2>& points);
	void _draw_bg(cv::Mat& preview_frame, const struct wii_info &wii);
};

} // ns impl

