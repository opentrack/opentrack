/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <array>
#include <opencv2/core.hpp>

namespace papertracker {
    /* Quality measures for a refined marker, taken over its worst edge.
    */
    struct marker_edge_quality {
        /* Largest mean squared distance (px^2) of an edge's subpixel edge points from its fitted line.
           High values indicate bent, occluded or otherwise non-straight edges.
        */
        double max_line_fit_mse = 0;

        /* Smallest fraction (0..1) of an edge's sample points whose peak gradient reached the minimum
           sample contrast. Low values indicate an edge that is partially occluded or out of frame.
        */
        double min_edge_coverage = 0;

        /* Smallest mean edge contrast (gray levels) across an edge, measured as the peak brightness
           difference between points 2 px apart straddling the edge.
        */
        double min_edge_contrast = 0;
    };

    bool refine_marker_edges(const cv::Mat &gray, std::array<cv::Point2f, 4> &corners, double range, double min_sample_contrast, marker_edge_quality *quality, int iterations = 1);
    bool marker_quad_angles_ok(const std::array<cv::Point2f, 4> &corners, double angle_limit_radians);
}
