/* Copyright (c) 2021 Michael Welter <michael@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "model_adapters.h"

#include "cv/video-widget.hpp"

#include <optional>

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>

namespace neuralnet_tracker_ns
{

/** Makes a maximum size cropping rect with the given aspect.
 *   @param aspect_w: nominator of the aspect ratio
 *   @param aspect_h: denom of the aspect ratio
 */
cv::Rect make_crop_rect_for_aspect(const cv::Size& size, int aspect_w, int aspect_h);

/** This class is responsible for drawing the debug/info gizmos
 *
 * In addition there function to transform the inputs to the size of
 * the preview image which can be different from the camera frame.
 */
class Preview
{
public:
    void init(const cv_video_widget& widget);
    void copy_video_frame(const cv::Mat& frame);
    void draw_gizmos(const std::optional<PoseEstimator::Face>& face,
                     const std::optional<cv::Rect2f>& last_roi,
                     const std::optional<cv::Rect2f>& last_localizer_roi,
                     const cv::Point2f& neckjoint_position);
    void overlay_netinput(const cv::Mat& netinput);
    void draw_fps(double fps, double last_inference_time);
    void copy_to_widget(cv_video_widget& widget);

private:
    // Transform from camera image to preview
    cv::Rect2f transform(const cv::Rect2f& r) const;
    cv::Point2f transform(const cv::Point2f& p) const;
    float transform(float s) const;

    cv::Mat preview_image_;
    cv::Size preview_size_ = { 0, 0 };
    float scale_ = 1.f;
    cv::Point2f offset_ = { 0.f, 0.f };
};

} // namespace neuralnet_tracker_ns