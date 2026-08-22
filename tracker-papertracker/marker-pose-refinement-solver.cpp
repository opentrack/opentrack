/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "marker-pose-refinement-solver.h"
#include <cmath>
#include <algorithm>

using namespace papertracker;

#if CV_MAJOR_VERSION >= 5

#include <opencv2/imgproc.hpp>

/* Pose refinement class with support for per-marker weights.
*/
WeightedMarkerPoseRefinementSolver::WeightedMarkerPoseRefinementSolver(
    const std::vector<cv::Vec3d> &object_points,
    const std::vector<cv::Vec2f> &image_points,
    const std::vector<double> &weights
) : object_points(object_points),
    image_points(image_points),
    weights(weights),
    max_points_seen(0)
{
    // Reserve space for temporary vectors.
    projected_points.reserve(object_points.size());
    vertex_weights_sqrt.reserve(object_points.size());

    // Levenberg-Marquardt solver.
    lm_solver = std::make_unique<cv::LevMarq>(6, std::ref(*this));
}

/* Callback used by cv::LevMarq class for pose refinement.
*/
bool WeightedMarkerPoseRefinementSolver::operator()(cv::InputOutputArray param, cv::OutputArray err, cv::OutputArray J) {
    // Rotation and translation vectors.
    const auto param_mat = param.getMat();

    cv::Vec3d rvec = { param_mat.at<double>(0), param_mat.at<double>(1), param_mat.at<double>(2) };
    cv::Vec3d tvec = { param_mat.at<double>(3), param_mat.at<double>(4), param_mat.at<double>(5) };

    const int object_points_size = (int) object_points.size();

    const bool need_jacobian = J.needed();

    if (need_jacobian) {
        // Project points and obtain Jacobian matrix derivatives.
        const int rows_needed = static_cast<int>(object_points.size() * 2);
        const int cols_needed = static_cast<int>(dist_coeffs.size() + 10);
        jacobian.create(rows_needed, cols_needed, CV_64F);

        cv::projectPoints(object_points, rvec, tvec, camera_matrix, dist_coeffs, projected_points, jacobian);

        // Copy computed derivatives to Jacobian matrix provided by cv::LevMarq class.
        J.create(static_cast<int>(max_points_seen * 2), 6, CV_64F);

        auto J_mat = J.getMat();
        for (int i = 0; i < object_points_size; ++i) {
            double w = vertex_weights_sqrt[i];
            for (int col = 0; col < 6; ++col) {
                J_mat.at<double>(2 * i,     col) = w * jacobian.at<double>(2 * i,     col);
                J_mat.at<double>(2 * i + 1, col) = w * jacobian.at<double>(2 * i + 1, col);
            }
        }

        // Zero out any remaining rows to avoid unnecessary resizing of the Jacobian matrix,
        // which for OpenCV types would require reallocation even when it grows smaller.
        if (object_points_size < max_points_seen)
            J_mat.rowRange(2 * object_points_size, J_mat.rows).setTo(cv::Scalar::all(0));
    } else {
        // Project points without computing Jacobian matrix derivatives.
        cv::projectPoints(object_points, rvec, tvec, camera_matrix, dist_coeffs, projected_points);
    }

    // Calculate projection errors / residuals.
    err.create(static_cast<int>(max_points_seen * 2), 1, CV_64F);

    auto err_mat = err.getMat();
    for (int i = 0; i < object_points_size; ++i) {
        double w = vertex_weights_sqrt[i];
        err_mat.at<double>(2 * i)     = w * (projected_points[i][0] - image_points[i][0]);
        err_mat.at<double>(2 * i + 1) = w * (projected_points[i][1] - image_points[i][1]);
    }

    // Zero out any remaining rows to avoid unnecessary resizing of the errors vector, which
    // for OpenCV types would require reallocation even when it grows smaller.
    if (object_points_size < max_points_seen)
        err_mat.rowRange(2 * object_points_size, err_mat.rows).setTo(cv::Scalar::all(0));

    return true;
}

/* Refine the pose specified by rvec/tvec using the points and marker weights passed in the constructor.
*/
bool WeightedMarkerPoseRefinementSolver::refineLeastSquares(cv::Vec3d &rvec, cv::Vec3d &tvec, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs) {
    // Update the maximum number of points seen so far to avoid unnecessary reallocations in the LevMarq callback.
    if (object_points.size() > max_points_seen)
        max_points_seen = object_points.size();

    // Compute the square root of all marker weights and assign them to each marker's vertices.
    vertex_weights_sqrt.clear();
    vertex_weights_sqrt.reserve(object_points.size());

    for (double w : weights) {
        double sqrt_w = std::sqrt(std::max(0.0, w));

        for (int k = 0; k < 4; ++k)
            vertex_weights_sqrt.push_back(sqrt_w);
    }

    // Clear and size projected points vector.
    projected_points.clear();
    projected_points.reserve(object_points.size());

    // Update camera matrix and distortion coefficients.
    this->camera_matrix = camera_matrix;

    this->dist_coeffs.reserve(dist_coeffs.size());
    this->dist_coeffs.assign(dist_coeffs.begin(), dist_coeffs.end());

    // Refine pose using Levenberg-Marquardt solver.
    cv::Mat_<double> pose = {
        rvec[0], rvec[1], rvec[2],
        tvec[0], tvec[1], tvec[2]
    };

    const auto report = lm_solver->run(pose);

    // Update pose rvec and tvec vectors.
    rvec[0] = pose(0);
    rvec[1] = pose(1);
    rvec[2] = pose(2);

    tvec[0] = pose(3);
    tvec[1] = pose(4);
    tvec[2] = pose(5);

    return report.found;
}

#else // CV_MAJOR_VERSION < 5
/* Dummy implementation for when OpenCV >= 5 is not available.
*/
WeightedMarkerPoseRefinementSolver::WeightedMarkerPoseRefinementSolver(
    const std::vector<cv::Vec3d> &object_points,
    const std::vector<cv::Vec2f> &image_points,
    const std::vector<double> &weights
) {}

bool WeightedMarkerPoseRefinementSolver::operator()(cv::InputOutputArray param, cv::OutputArray err, cv::OutputArray J) {
    return false;
}

bool WeightedMarkerPoseRefinementSolver::refineLeastSquares(cv::Vec3d& rvec, cv::Vec3d& tvec, const cv::Matx33d& camera_matrix, const std::vector<double>& dist_coeffs)
{
    return false;
}
#endif
