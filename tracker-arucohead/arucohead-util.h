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

namespace arucohead {
    cv::Vec3d average_rotation(std::vector<cv::Vec3d> &rvecs);
    cv::Vec3d average_rotation(std::unordered_map<int, cv::Vec3d> &rvecs);
    cv::Vec3d average_rotation(std::unordered_map<int, cv::Vec3d> &rvecs, int exclude_id);
    cv::Vec3d average_translation(std::vector<cv::Vec3d> &tvecs);
    cv::Vec3d average_translation(std::unordered_map<int, cv::Vec3d> &tvecs);
    cv::Vec3d average_translation(std::unordered_map<int, cv::Vec3d> &tvecs, int exclude_id);
    double circumference_to_radius(double circumference);
    cv::Vec3d circle_edge_intersection(double radius, cv::Vec3d &direction);
    cv::Vec3d get_xz_direction_vector(cv::Vec3d &rvec);
    cv::Vec4d rotation_matrix_to_quaternion(const cv::Mat &R);
    cv::Mat quaternion_to_rotation_matrix(const cv::Vec4d &quat);
    cv::Vec4d quaternion_multiply(const cv::Vec4d &q1, const cv::Vec4d &q2);
    cv::Vec3d rotation_matrix_to_euler_zyx(const cv::Mat &R);
    cv::Vec3d shrink_tvec(const cv::Vec3d &tvec, double xz_reference, double y_reference);
    cv::Vec3d expand_tvec(const cv::Vec3d &tvec, double xz_reference, double y_reference);
    std::pair<cv::Vec3d, cv::Vec3d> get_marker_local_transform(const cv::Vec3d &rvec_measured, const cv::Vec3d &tvec_measured, const cv::Vec3d &pose_rvec, const cv::Vec3d &pose_tvec, double xz_reference, double y_reference);
    double get_marker_z_angle(const cv::Vec3d &rvec);
    double angle_between_rotations(const cv::Vec3d &rvec1, const cv::Vec3d &rvec2);
    bool marker_has_flipped(cv::Vec3d previous_rvec, cv::Vec3d& rvec);
}
