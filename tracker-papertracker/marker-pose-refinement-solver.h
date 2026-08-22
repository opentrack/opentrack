/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <opencv2/core.hpp>
#include <vector>

#if CV_MAJOR_VERSION >= 5
#include <opencv2/geometry/3d.hpp>
#include <memory>
#endif

namespace papertracker {
    class WeightedMarkerPoseRefinementSolver {
    public:
        WeightedMarkerPoseRefinementSolver(
            const std::vector<cv::Vec3d> &object_points,
            const std::vector<cv::Vec2f> &image_points,
            const std::vector<double> &marker_weights
        );

        bool operator()(cv::InputOutputArray param, cv::OutputArray err, cv::OutputArray J);
        bool refineLeastSquares(cv::Vec3d &rvec, cv::Vec3d &tvec, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs);

    #if CV_MAJOR_VERSION >= 5
    private:
        // external references
        const std::vector<cv::Vec3d> &object_points;
        const std::vector<cv::Vec2f> &image_points;
        const std::vector<double> &weights;

        // internal variables
        cv::Matx33d camera_matrix;
        std::vector<double> dist_coeffs;
        std::vector<double> vertex_weights_sqrt;
        std::vector<cv::Vec2d> projected_points;
        size_t max_points_seen;
        std::unique_ptr<cv::LevMarq> lm_solver;
        cv::Mat jacobian;
    #endif
    };
}