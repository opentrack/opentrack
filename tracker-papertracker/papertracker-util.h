/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <vector>
#include <unordered_map>
#include <opencv2/core.hpp>

namespace papertracker {
    cv::Vec3d average_rotation(const std::vector<cv::Vec3d> &rvecs, const std::vector<double> *normalized_weights = nullptr);
    cv::Vec3d average_translation(const std::vector<cv::Vec3d> &tvecs, const std::vector<double> *normalized_weights = nullptr);
    double circumference_to_radius(double circumference);
    cv::Vec2d circle_edge_intersection(double radius, const cv::Vec2d &direction);
    cv::Vec2d get_xz_direction_vector(const cv::Vec3d &rvec);
    cv::Vec4d rotation_matrix_to_quaternion(const cv::Matx33d &R);
    cv::Matx33d quaternion_to_rotation_matrix(const cv::Vec4d &quat);
    cv::Vec3d rotation_matrix_to_euler_zyx(const cv::Matx33d &R);
    std::pair<cv::Vec3d, cv::Vec3d> get_marker_local_transform(const cv::Vec3d &rvec_measured, const cv::Vec3d &tvec_measured, const cv::Vec3d &pose_rvec, const cv::Vec3d &pose_tvec);
    double get_marker_relative_angle(const cv::Matx33d &R, const cv::Vec3d &reference);
    double get_marker_relative_angle(const cv::Vec3d &rvec, const cv::Vec3d &reference);
    double angle_between_rotations(const cv::Vec3d &rvec1, const cv::Vec3d &rvec2);
    float get_marker_line_of_symmetry(const std::vector<std::array<cv::Point2f, 4>> &markers);
    bool vertical_line_intersects_marker(const float line_x, const std::array<cv::Point2f, 4> &corners);
}
