/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "marker-pair-refinement-solver.h"

using namespace papertracker;

#if CV_MAJOR_VERSION >= 5

/* Relative pose refinement class for parent/child marker pairs.
*/
MarkerPairRefinementSolver::MarkerPairRefinementSolver(int max_samples_per_pair) : 
    current_pair(0, 0), 
    max_samples_per_pair(max_samples_per_pair),
    max_samples_seen(0)
{
    // Levenberg-Marquardt solver.
    lm_solver = std::make_unique<cv::LevMarq>(6, std::ref(*this));

    // Reserve enough space for 16 * 15 marker pairs.
    pair_samples.reserve(16 * 15);
}

/* Callback used by cv::LevMarq for relative pose refinement.
*/
bool MarkerPairRefinementSolver::operator()(cv::InputOutputArray param, cv::OutputArray err, cv::OutputArray J) {
    // Generic marker object points.
    const std::array<cv::Vec3d, 4> object_points = {
        cv::Vec3d(-marker_size_cm / 2.0,  marker_size_cm / 2.0, 0.0),
        cv::Vec3d(marker_size_cm / 2.0,  marker_size_cm / 2.0, 0.0),
        cv::Vec3d(marker_size_cm / 2.0, -marker_size_cm / 2.0, 0.0),
        cv::Vec3d(-marker_size_cm / 2.0, -marker_size_cm / 2.0, 0.0)
    };

    // Relative rotation and translation vectors.
    const auto param_mat = param.getMat();

    cv::Vec3d relative_rvec = { param_mat.at<double>(0), param_mat.at<double>(1), param_mat.at<double>(2) };
    cv::Vec3d relative_tvec = { param_mat.at<double>(3), param_mat.at<double>(4), param_mat.at<double>(5) };

    // Retrieve samples for current marker pair.
    const auto samples = pair_samples[current_pair];

    const auto num_samples = samples.size();
    if (num_samples == 0)
        return false;

    max_samples_seen = std::max(max_samples_seen, static_cast<int>(num_samples));

    // Initialize Jacobian and error matrices.
    cv::Mat J_mat;
    bool needs_jacobian = J.needed();
    if (needs_jacobian) {
        J.create(static_cast<int>(4 * 2 * max_samples_seen), 6, CV_64F);
        J_mat = J.getMat();

        jacobian.create(8, 6, CV_64F);
    }

    err.create(static_cast<int>(4 * 2 * max_samples_seen), 1, CV_64F);
    auto err_mat = err.getMat();

    // Calculate Jacobian and error matrices.
    std::array<cv::Point2d, 4> projected_points;
    cv::Mat projected_points_mat(static_cast<int>(projected_points.size()), 1, CV_64FC2, projected_points.data());

    for (int si = 0; si < num_samples; ++si) {
        cv::Vec3d sample_rvec;
        cv::Vec3d sample_tvec;

        if (needs_jacobian) {
            // Determine the child's camera-space pose for the current sample.
            cv::Matx33d dr3dr1;
            cv::Matx33d dr3dt1;
            cv::Matx33d dt3dr1;
            cv::Matx33d dt3dt1;
            cv::composeRT(relative_rvec, relative_tvec, samples[si].parent_rvec, samples[si].parent_tvec, sample_rvec, sample_tvec, dr3dr1, dr3dt1, cv::noArray(), cv::noArray(), dt3dr1, dt3dt1);

            // 6x6 matrix of derivatives obtained from composeRT.
            cv::Matx66d compose_mat = cv::Matx66d::zeros();
            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; c++) {
                    compose_mat(r,   c)   = dr3dr1(r, c); // d(sample_rvec) / d(relative_rvec)
                    compose_mat(r,   c+3) = dr3dt1(r, c); // d(sample_rvec) / d(relative_tvec)
                    compose_mat(r+3, c)   = dt3dr1(r, c); // d(sample_tvec) / d(relative_rvec)
                    compose_mat(r+3, c+3) = dt3dt1(r, c); // d(sample_tvec) / d(relative_tvec)
                }
            }

            // Project the child marker's vertices and obtain Jacobian matrix derivatives.
            cv::projectPoints(object_points, sample_rvec, sample_tvec, camera_matrix, dist_coeffs, projected_points_mat, jacobian);

            // Multiply the first six columns of the Jacobian matrix obtained from projectPoints by a matrix
            // containing the derivatives obtained from composeRT:
            //
            //     d(projected_points)         d(sample_rvec, sample_tvec)            d(projected_points)
            // --------------------------- * ------------------------------- == ------------------------------- 
            // d(sample_rvec, sample_tvec)   d(relative_rvec, relative_tvec)    d(relative_rvec, relative_tvec)
            //
            jacobian_output_block.create(jacobian.rows, 6, CV_64F);

            cv::Mat jacobian_submatrix = jacobian(cv::Range::all(), cv::Range(0, 6));
            cv::gemm(jacobian_submatrix, compose_mat, 1, cv::noArray(), 0, jacobian_output_block);

            // Copy the computed derivatives to the Jacobian matrix provided by cv::LevMarq class.
            for (int row = 0; row < jacobian_output_block.rows; ++row)
                for (int col = 0; col < jacobian_output_block.cols; ++col)
                    J_mat.at<double>(4 * 2 * si + row, col) = jacobian_output_block.at<double>(row, col);
        } else {
            // Determine the child's camera-space pose for the current sample and project its vertices.
            cv::composeRT(relative_rvec, relative_tvec, samples[si].parent_rvec, samples[si].parent_tvec, sample_rvec, sample_tvec);
            cv::projectPoints(object_points, sample_rvec, sample_tvec, camera_matrix, dist_coeffs, projected_points_mat);
        }

        // Calculate projection errors / residuals.
        for (int pi = 0; pi < 4; ++pi) {
            err_mat.at<double>(4 * 2 * si + 2 * pi)     = projected_points[pi].x - samples[si].image_points[pi].x;
            err_mat.at<double>(4 * 2 * si + 2 * pi + 1) = projected_points[pi].y - samples[si].image_points[pi].y;
        }
    }

    // Zero out any remaining rows to avoid unnecessary resizing of the Jacobian and error
    // matrices, which for OpenCV types would require reallocation even when it grows smaller.
    if (num_samples < max_samples_seen) {
        if (needs_jacobian)
            J_mat.rowRange(static_cast<int>(4 * 2 * num_samples), J_mat.rows).setTo(cv::Scalar::all(0));

        err_mat.rowRange(static_cast<int>(4 * 2 * num_samples), err_mat.rows).setTo(cv::Scalar::all(0));
    }
    
    return true;
}

/* Add marker pair sample, up to max_samples_per_pair samples for each pair.
*/
bool MarkerPairRefinementSolver::add_pair_observation(int parent_id, int child_id, const cv::Vec3d &parent_rvec, const cv::Vec3d &parent_tvec, const std::array<cv::Point2f, 4> &child_image_points, bool evict_oldest) {
    const auto pair = std::pair<int,int>(parent_id, child_id);

    // Remove oldest entry if full and evict_oldest is set.
    while (evict_oldest && pair_samples[pair].size() >= max_samples_per_pair)
        pair_samples.erase(pair_samples.begin());

    // Insert sample if there's space available.
    if (pair_samples[pair].size() < max_samples_per_pair) {
        PairInfo info;
        info.parent_rvec = parent_rvec;
        info.parent_tvec = parent_tvec;
        info.image_points = child_image_points;

        pair_samples[pair].reserve(max_samples_per_pair);
        pair_samples[pair].push_back(info);

        return true;
    }

    return false;
}

/* Refine the pose of the given child marker relative to the given parent marker.
*/
bool MarkerPairRefinementSolver::refine_pair(int parent_id, int child_id, cv::Vec3d &relative_rvec, cv::Vec3d &relative_tvec, double marker_size_cm, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs) {
    // Set the parent/child pair whose relative pose is to be refined.
    current_pair = std::pair<int, int>(parent_id, child_id);

    // Set the marker size.
    this->marker_size_cm = marker_size_cm;

    // Update camera matrix and distortion coefficients.
    this->camera_matrix = camera_matrix;

    this->dist_coeffs.reserve(dist_coeffs.size());
    this->dist_coeffs.assign(dist_coeffs.begin(), dist_coeffs.end());

    // Refine relative pose using Levenberg-Marquardt solver.
    cv::Mat_<double> relative_pose = {
        relative_rvec[0], relative_rvec[1], relative_rvec[2],
        relative_tvec[0], relative_tvec[1], relative_tvec[2]
    };
    
    const auto report = lm_solver->run(relative_pose);

    // Update relative pose rvec and tvec vectors.
    relative_rvec[0] = relative_pose(0);
    relative_rvec[1] = relative_pose(1);
    relative_rvec[2] = relative_pose(2);

    relative_tvec[0] = relative_pose(3);
    relative_tvec[1] = relative_pose(4);
    relative_tvec[2] = relative_pose(5);

    return report.found;
}

#else CV_MAJOR_VERSION < 5
/* Dummy implementation for when OpenCV >= 5 is not available.
*/
MarkerPairRefinementSolver::MarkerPairRefinementSolver(int max_samples_per_pair)
{}

bool MarkerPairRefinementSolver::operator()(cv::InputOutputArray param, cv::OutputArray err, cv::OutputArray J) {
    return false;
}

bool MarkerPairRefinementSolver::add_pair_observation(int parent_id, int child_id, const cv::Vec3d &parent_rvec, const cv::Vec3d &parent_tvec, const std::array<cv::Point2f, 4> &child_image_points, bool evict_oldest) {
    return false;
}

bool MarkerPairRefinementSolver::refine_pair(int parent_id, int child_id, cv::Vec3d &relative_rvec, cv::Vec3d &relative_tvec, double marker_size_cm, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs) {
    return false;
}
#endif
