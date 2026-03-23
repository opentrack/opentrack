/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "arucohead-util.h"
#include "config.h"
#include <opencv2/opencv.hpp>
#include <math.h>

namespace arucohead {
    /* Compute average rotation from an std::vector of rotation vectors.
    */
    cv::Vec3d average_rotation(std::vector<cv::Vec3d> &rvecs)
    {
        if (rvecs.size() == 1)
            return rvecs[0];

        std::vector<cv::Vec4d> quaternions;

        for (size_t i = 0; i < rvecs.size(); ++i) {
            cv::Mat R;
            cv::Rodrigues(rvecs[i], R);
            quaternions.push_back(rotation_matrix_to_quaternion(R));
        }

        cv::Vec4d q_avg = quaternions[0];
        for (size_t i = 1; i < quaternions.size(); ++i) {
            if (q_avg.dot(quaternions[i]) < 0)
                quaternions[i] = -quaternions[i];
            q_avg += quaternions[i];
        }

        q_avg /= (double)quaternions.size();
        q_avg /= cv::norm(q_avg);

        auto R_avg = quaternion_to_rotation_matrix(q_avg);

        cv::Vec3d rvec;
        cv::Rodrigues(R_avg, rvec);

        return rvec;
    }

    /* Compute average rotation from an unordered map of rotation vectors.
    */
    cv::Vec3d average_rotation(std::unordered_map<int, cv::Vec3d> &rvecs)
    {
        std::vector<cv::Vec3d> vectors;

        for (auto p : rvecs)
            vectors.push_back(p.second);

        return average_rotation(vectors);
    }

    /* Compute average rotation from an unordered map of rotation vectors, excluding specified ID.
    */
    cv::Vec3d average_rotation(std::unordered_map<int, cv::Vec3d> &rvecs, int exclude_id)
    {
        std::vector<cv::Vec3d> vectors;

        for (auto p : rvecs)
            if (p.first != exclude_id)
                vectors.push_back(p.second);

        return average_rotation(vectors);
    }

    /* Compute average translation from an std::vector of translation vectors.
    */
    cv::Vec3d average_translation(std::vector<cv::Vec3d> &tvecs)
    {
        if (tvecs.size() == 1)
            return tvecs[0];

        double avg_x = 0;
        double avg_y = 0;
        double avg_z = 0;

        for (size_t v = 0; v < tvecs.size(); ++v) {
            avg_x += tvecs[v][0];
            avg_y += tvecs[v][1];
            avg_z += tvecs[v][2];
        }

        avg_x /= (double)tvecs.size();
        avg_y /= (double)tvecs.size();
        avg_z /= (double)tvecs.size();

        return { avg_x, avg_y, avg_z };
    }

    /* Compute average translation from an unordered map of translation vectors.
    */
    cv::Vec3d average_translation(std::unordered_map<int, cv::Vec3d> &tvecs)
    {
        std::vector<cv::Vec3d> vectors;

        for (auto p : tvecs)
            vectors.push_back(p.second);

        return average_translation(vectors);
    }

    /* Compute average translation from an unordered map of translation vectors, excluding specified id.
    */
    cv::Vec3d average_translation(std::unordered_map<int, cv::Vec3d> &tvecs, int exclude_id)
    {
        std::vector<cv::Vec3d> vectors;

        for (auto p : tvecs)
            if (p.first != exclude_id)
                vectors.push_back(p.second);

        return average_translation(vectors);
    }

    /* Compute radius from given circumference.
    */
    double circumference_to_radius(double circumference)
    {
        return circumference / (2.0 * CV_PI);
    }

    /* Find the (X, Z) intersection between a direction vector's projection onto the XZ plane
       and a circle lying on the XZ plane with its origin at (0, 0).
     */
    cv::Vec3d circle_edge_intersection(double radius, cv::Vec3d &direction)
    {
        double magnitude = cv::sqrt(cv::pow(direction[0], 2) + cv::pow(direction[1], 2));
        if (magnitude == 0)
            return cv::Mat();

        double nx = direction[0] / magnitude;
        double nz = direction[1] / magnitude;

        return { nx * radius, nz * radius };
    }

    /* Rotate axis (0, 0, -1) by rvec and project onto XZ plane.
    */
    cv::Vec3d get_xz_direction_vector(cv::Vec3d &rvec)
    {
        cv::Mat R;
        cv::Rodrigues(rvec, R);

        cv::Vec3d v = { 0, 0, -1 };
        cv::Mat vt = R * v;

        double norm = cv::norm(vt);

        if (norm != 0)
            vt = vt / cv::norm(vt);

        return cv::Vec3d(vt.at<double>(0), vt.at<double>(2));
    }

    /* Convert rotation matrix to quaternion.
    */
    cv::Vec4d rotation_matrix_to_quaternion(const cv::Mat &R)
    {
        const auto trace = cv::trace(R);

        double s;
        double w;
        double x;
        double y;
        double z;

        if (trace[0] > 0.0) {
            s = 0.5 / cv::sqrt(trace[0] + 1.0);
            w = 0.25 / s;
            x = (R.at<double>(2, 1) - R.at<double>(1, 2)) * s;
            y = (R.at<double>(0, 2) - R.at<double>(2, 0)) * s;
            z = (R.at<double>(1, 0) - R.at<double>(0, 1)) * s;
        } else if (R.at<double>(0, 0) > R.at<double>(1, 1) && R.at<double>(0, 0) > R.at<double>(2, 2)) {
            s = 2.0 * cv::sqrt(1.0 + R.at<double>(0, 0) - R.at<double>(1, 1) - R.at<double>(2, 2));
            w = (R.at<double>(2, 1) - R.at<double>(1, 2)) / s;
            x = 0.25 * s;
            y = (R.at<double>(0, 1) + R.at<double>(1, 0)) / s;
            z = (R.at<double>(0, 2) + R.at<double>(2, 0)) / s;
        } else if (R.at<double>(1, 1) > R.at<double>(2, 2)) {
            s = 2.0 * cv::sqrt(1.0 + R.at<double>(1, 1) - R.at<double>(0, 0) - R.at<double>(2, 2));
            w = (R.at<double>(0, 2) - R.at<double>(2, 0)) / s;
            x = (R.at<double>(0, 1) + R.at<double>(1, 0)) / s;
            y = 0.25 * s;
            z = (R.at<double>(1, 2) + R.at<double>(2, 1)) / s;
        } else {
            s = 2.0 * cv::sqrt(1.0 + R.at<double>(2, 2) - R.at<double>(0, 0) - R.at<double>(1, 1));
            w = (R.at<double>(1, 0) - R.at<double>(0, 1)) / s;
            x = (R.at<double>(0, 2) + R.at<double>(2, 0)) / s;
            y = (R.at<double>(1, 2) + R.at<double>(2, 1)) / s;
            z = 0.25 * s;
        }

        return { w, x, y, z };
    }

    /* Convert quaternion to rotation matrix.
    */
    cv::Mat quaternion_to_rotation_matrix(const cv::Vec4d &quat) {
        const double w = quat[0];
        const double x = quat[1];
        const double y = quat[2];
        const double z = quat[3];

        double data[9] = {
            1 - 2*y*y - 2*z*z,     2*x*y - 2*w*z,     2*x*z + 2*w*y,
                2*x*y + 2*w*z, 1 - 2*x*x - 2*z*z,     2*y*z - 2*w*x,
                2*x*z - 2*w*y,     2*y*z + 2*w*x, 1 - 2*x*x - 2*y*y
        };

        return cv::Mat(3, 3, CV_64F, data).clone();
    }

    cv::Vec4d quaternion_multiply(const cv::Vec4d &q1, const cv::Vec4d &q2) {
        const double w = q1[0]*q2[0] - q1[1]*q2[1] - q1[2]*q2[2] - q1[3]*q2[3];
        const double x = q1[0]*q2[1] + q1[1]*q2[0] + q1[2]*q2[3] - q1[3]*q2[2];
        const double y = q1[0]*q2[2] - q1[1]*q2[3] + q1[2]*q2[0] + q1[3]*q2[1];
        const double z = q1[0]*q2[3] + q1[1]*q2[2] - q1[2]*q2[1] + q1[3]*q2[0];

        return { w, x, y, z };
    }

    /* Convert rotation matrix to euler angles (ZYX).
    */
    cv::Vec3d rotation_matrix_to_euler_zyx(const cv::Mat &R) {
        const double r00 = R.at<double>(0, 0);
        const double r10 = R.at<double>(1, 0);
        const double r11 = R.at<double>(1, 1);
        const double r12 = R.at<double>(1, 2);
        const double r20 = R.at<double>(2, 0);
        const double r21 = R.at<double>(2, 1);
        const double r22 = R.at<double>(2, 2);

        double sy = sqrt(r00*r00 + r10*r10);

        double roll;
        double pitch;
        double yaw;

        if (sy > 1e-6) {
            roll = atan2(r21, r22);
            pitch = atan2(-r20, sy);
            yaw = atan2(r10, r00);
        } else {
            roll = atan2(-r12, r11);
            pitch = atan2(-r20, sy);
            yaw = 0.0;
        }

        return { roll, pitch, yaw };
    }

    /* Divide vector by (xz_reference, y_reference, xz_reference).
    */
    cv::Vec3d shrink_tvec(const cv::Vec3d &tvec, double xz_reference, double y_reference) {
        return cv::Vec3d(tvec[0] / xz_reference, tvec[1] / y_reference, tvec[2] / y_reference);
    }

    /* Multiply vector by (xz_reference, y_reference, xz_reference).
    */
    cv::Vec3d expand_tvec(const cv::Vec3d &tvec, double xz_reference, double y_reference) {
        return cv::Vec3d(tvec[0] * xz_reference, tvec[1] * y_reference, tvec[2] * y_reference);
    }

    /* Get marker's local transform relative to a reference head pose.
    */
    std::pair<cv::Vec3d, cv::Vec3d> get_marker_local_transform(const cv::Vec3d &rvec_measured, const cv::Vec3d &tvec_measured, const cv::Vec3d &pose_rvec, const cv::Vec3d &pose_tvec, double xz_reference, double y_reference) {
        cv::Vec3d rvec_local;
        cv::Vec3d tvec_local;

        cv::Mat R_body_to_camera;
        cv::Rodrigues(pose_rvec, R_body_to_camera);

        cv::Mat R_marker_to_camera;
        cv::Rodrigues(rvec_measured, R_marker_to_camera);

        cv::Rodrigues(R_body_to_camera.inv() * R_marker_to_camera, rvec_local);

        cv::Vec3d tvec_relative(3);
        tvec_relative[0] = tvec_measured[0] - pose_tvec[0];
        tvec_relative[1] = tvec_measured[1] - pose_tvec[1];
        tvec_relative[2] = tvec_measured[2] - pose_tvec[2];

        cv::Mat tvec_local_mat = R_body_to_camera.inv() * cv::Mat(tvec_relative);
        tvec_local = tvec_local_mat.reshape(1, 1);

        return {rvec_local, shrink_tvec(tvec_local, xz_reference, y_reference)};
    }

    double angle_between_rotations(const cv::Vec3d &v1, const cv::Vec3d &v2) {
        cv::Mat R1;
        cv::Rodrigues(v1, R1);

        cv::Mat R2;
        cv::Rodrigues(v2, R2);

        const auto q1 = rotation_matrix_to_quaternion(R1);
        const auto q2 = rotation_matrix_to_quaternion(R2);

        const auto q_diff = quaternion_multiply(q2, cv::Vec4d(q1[0], -q1[1], -q1[2], -q1[3]));

        return acos(std::min(fabs(q_diff[0]), 1.0)) * 2.0;
    }

    bool marker_has_flipped(cv::Vec3d previous_rvec, cv::Vec3d &rvec) {
        return angle_between_rotations(rvec, previous_rvec) > CV_PI / 180.0 * ARUCOHEAD_MARKER_FLIP_DETECTION_THRESHOLD;
    }
}