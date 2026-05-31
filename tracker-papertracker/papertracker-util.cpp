/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "papertracker-util.h"
#include "config.h"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <optional>

namespace papertracker {
    /* Compute average rotation from an std::vector of rotation vectors.
    */
    cv::Vec3d average_rotation(const std::vector<cv::Vec3d> &rvecs)
    {
        if (rvecs.size() == 1)
            return rvecs[0];

        cv::Matx33d R;
        cv::Rodrigues(rvecs[0], R);
        cv::Vec4d q_avg = rotation_matrix_to_quaternion(R);

        for (size_t i = 1; i < rvecs.size(); ++i) {
            cv::Rodrigues(rvecs[i], R);
            cv::Vec4d quat = rotation_matrix_to_quaternion(R);

            if (q_avg.dot(quat) < 0)
                quat = -quat;

            q_avg += quat;
        }

        q_avg /= cv::norm(q_avg);

        auto R_avg = quaternion_to_rotation_matrix(q_avg);

        cv::Vec3d rvec;
        cv::Rodrigues(R_avg, rvec);

        return rvec;
    }

    /* Compute average translation from an std::vector of translation vectors.
    */
    cv::Vec3d average_translation(const std::vector<cv::Vec3d> &tvecs)
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

    /* Compute radius from given circumference.
    */
    double circumference_to_radius(double circumference)
    {
        return circumference / (2.0 * CV_PI);
    }

    /* Find the (X, Z) intersection between a direction vector's projection onto the XZ plane
       and a circle lying on the XZ plane with its origin at (0, 0).
     */
    cv::Vec2d circle_edge_intersection(double radius, const cv::Vec2d &direction)
    {
        double magnitude = cv::sqrt(cv::pow(direction[0], 2) + cv::pow(direction[1], 2));
        if (magnitude == 0)
            return cv::Vec2d();

        double nx = direction[0] / magnitude;
        double nz = direction[1] / magnitude;

        return { nx * radius, nz * radius };
    }

    /* Rotate axis (0, 0, -1) by rvec and project onto XZ plane.
    */
    cv::Vec2d get_xz_direction_vector(const cv::Vec3d &rvec)
    {
        cv::Matx33d R;
        cv::Rodrigues(rvec, R);

        cv::Vec3d v = { 0, 0, -1 };
        cv::Vec3d vt = R * v;

        double norm = cv::norm(vt);

        if (norm != 0)
            vt = vt / cv::norm(vt);

        return cv::Vec2d(vt(0), vt(2));
    }

    /* Convert rotation matrix to quaternion.
    */
    cv::Vec4d rotation_matrix_to_quaternion(const cv::Matx33d &R)
    {
        const auto trace = cv::trace(R);

        double s;
        double w;
        double x;
        double y;
        double z;

        if (trace > 0.0) {
            s = 0.5 / cv::sqrt(trace + 1.0);
            w = 0.25 / s;
            x = (R(2, 1) - R(1, 2)) * s;
            y = (R(0, 2) - R(2, 0)) * s;
            z = (R(1, 0) - R(0, 1)) * s;
        } else if (R(0, 0) > R(1, 1) && R(0, 0) > R(2, 2)) {
            s = 2.0 * cv::sqrt(1.0 + R(0, 0) - R(1, 1) - R(2, 2));
            w = (R(2, 1) - R(1, 2)) / s;
            x = 0.25 * s;
            y = (R(0, 1) + R(1, 0)) / s;
            z = (R(0, 2) + R(2, 0)) / s;
        } else if (R(1, 1) > R(2, 2)) {
            s = 2.0 * cv::sqrt(1.0 + R(1, 1) - R(0, 0) - R(2, 2));
            w = (R(0, 2) - R(2, 0)) / s;
            x = (R(0, 1) + R(1, 0)) / s;
            y = 0.25 * s;
            z = (R(1, 2) + R(2, 1)) / s;
        } else {
            s = 2.0 * cv::sqrt(1.0 + R(2, 2) - R(0, 0) - R(1, 1));
            w = (R(1, 0) - R(0, 1)) / s;
            x = (R(0, 2) + R(2, 0)) / s;
            y = (R(1, 2) + R(2, 1)) / s;
            z = 0.25 * s;
        }

        return { w, x, y, z };
    }

    /* Convert quaternion to rotation matrix.
    */
    cv::Matx33d quaternion_to_rotation_matrix(const cv::Vec4d &quat) {
        const double w = quat[0];
        const double x = quat[1];
        const double y = quat[2];
        const double z = quat[3];

        double data[9] = {
            1 - 2*y*y - 2*z*z,     2*x*y - 2*w*z,     2*x*z + 2*w*y,
                2*x*y + 2*w*z, 1 - 2*x*x - 2*z*z,     2*y*z - 2*w*x,
                2*x*z - 2*w*y,     2*y*z + 2*w*x, 1 - 2*x*x - 2*y*y
        };

        return cv::Matx33d(data);
    }

    /* Convert rotation matrix to euler angles (ZYX).
    */
    cv::Vec3d rotation_matrix_to_euler_zyx(const cv::Matx33d &R) {
        const double r00 = R(0, 0);
        const double r10 = R(1, 0);
        const double r11 = R(1, 1);
        const double r12 = R(1, 2);
        const double r20 = R(2, 0);
        const double r21 = R(2, 1);
        const double r22 = R(2, 2);

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

    /* Get marker's local transform relative to a reference head pose.
    */
    std::pair<cv::Vec3d, cv::Vec3d> get_marker_local_transform(const cv::Vec3d &rvec_measured, const cv::Vec3d &tvec_measured, const cv::Vec3d &pose_rvec, const cv::Vec3d &pose_tvec) {
        cv::Matx33d R_body_to_camera;
        cv::Rodrigues(pose_rvec, R_body_to_camera);

        cv::Matx33d R_marker_to_camera;
        cv::Rodrigues(rvec_measured, R_marker_to_camera);

        const cv::Vec3d rvec_local;
        cv::Rodrigues(R_body_to_camera.inv() * R_marker_to_camera, rvec_local);

        cv::Vec3d tvec_relative(3);
        tvec_relative[0] = tvec_measured[0] - pose_tvec[0];
        tvec_relative[1] = tvec_measured[1] - pose_tvec[1];
        tvec_relative[2] = tvec_measured[2] - pose_tvec[2];

        const cv::Vec3d tvec_local = R_body_to_camera.inv() * tvec_relative;

        return {rvec_local, tvec_local};
    }

    /* Get the angle between a marker and the camera's Z axis given the marker's rotation matrix.
    */
    double get_marker_z_angle(const cv::Matx33d &R) {
        const cv::Vec3d marker_z(R(0, 2), R(1, 2), R(2, 2));
        const cv::Vec3d camera_z(0.0, 0.0, -1.0);

        const double cosine = std::clamp(marker_z.dot(camera_z), -1.0, 1.0);
        const double angle = std::acos(cosine);

        return angle;
    }

    /* Get the angle between a marker and the camera's Z axis given the marker's rotation vector.
    */
    double get_marker_z_angle(const cv::Vec3d &rvec) {
        cv::Matx33d R;
        cv::Rodrigues(rvec, R);

        return get_marker_z_angle(R);
    }

    /* Get the angle between rotations for a pair of rotation vectors.
    */
    double angle_between_rotations(const cv::Vec3d &v1, const cv::Vec3d &v2) {
        cv::Matx33d R1, R2;
        cv::Rodrigues(v1, R1);
        cv::Rodrigues(v2, R2);

        const cv::Matx33d R_diff = R1.t() * R2;
        const double trace = R_diff(0,0) + R_diff(1,1) + R_diff(2,2);
        const double cos_angle = std::clamp((trace - 1.0) / 2.0, -1.0, 1.0);

        return acos(cos_angle);
    }

    /* Get a tight bounding box for a set of markers.
    */
    cv::Rect2f get_marker_bounding_box(const std::vector<std::array<cv::Point2f, 4>> &markers)
    {
        if (markers.size() == 0 || markers[0].size() == 0)
            return cv::Rect2f(0, 0, 0, 0);

        cv::Point2f min = markers[0][0];
        cv::Point2f max = markers[0][0];

        for (const auto &marker : markers) {
            for (const auto &corner : marker) {
                min.x = std::min(corner.x, min.x);
                min.y = std::min(corner.y, min.y);
                max.x = std::max(corner.x, max.x);
                max.y = std::max(corner.y, max.y);
            }
        }

        return cv::Rect2f(min, max);
    }

    /* Count the number of pixels in common between two sides of an image when
       one side is folded over the other across a given line.
    */
    int symmetry_score(const cv::Mat& image, int line_of_symmetry_x) {
        const int image_width = image.cols;
        const int image_height = image.rows;

        const int left_width = line_of_symmetry_x;
        const int right_width = image_width - line_of_symmetry_x;
        const int smaller_width = std::min(left_width, right_width);

        // One side needs to be at least 1 pixel wide.
        if (smaller_width <= 0)
            return 0;

        // Crop the image into two equal halves split along the line of symmetry.
        cv::Mat left = image(cv::Rect(line_of_symmetry_x - smaller_width, 0, smaller_width, image_height));
        cv::Mat right = image(cv::Rect(line_of_symmetry_x, 0, smaller_width, image_height));

        // Flip the left half horizontally so it mirrors onto the right
        cv::Mat flipped_left;
        cv::flip(left, flipped_left, 1);

        // Create binary mask for the left half.
        cv::Mat left_mask;
        if (flipped_left.channels() == 3)
            cv::cvtColor(flipped_left, left_mask,  cv::COLOR_BGR2GRAY);
        else
            left_mask = flipped_left.clone();

        cv::threshold(left_mask, left_mask,  0, 255, cv::THRESH_BINARY);

        // Create binary mask for the right half.
        cv::Mat right_mask;
        if (right.channels() == 3)
            cv::cvtColor(right, right_mask, cv::COLOR_BGR2GRAY);
        else
            right_mask = right.clone();

        cv::threshold(right_mask, right_mask, 0, 255, cv::THRESH_BINARY);

        // Bitwise AND the two halves.
        cv::Mat overlap;
        cv::bitwise_and(left_mask, right_mask, overlap);

        // Count and return the number of matching pixels.
        return cv::countNonZero(overlap);
    }

    /* Find the vertical line that divides the detected markers into roughly symmetrical
       halves. This is usually the midpoint of their bounding box, but not always. If a
       marker at either end of a set is undetected for any reason the bounding box
       midpoint will no longer represent the marker set's actual midpoint. This algorithm
       can, at least in some cases, give a better estimate than the naive approach.
    */
    float get_marker_line_of_symmetry(const std::vector<std::array<cv::Point2f, 4>> &markers) {
        if (markers.size() == 0)
            return 0;

        // Operate inside the confines of the marker set's bounding box.
        auto rect = get_marker_bounding_box(markers);

        // Collect the minimum and maximum X values for each marker. The midpoints between
        // these extents represent potential lines of symmetry for the marker set.
        std::vector<float> extents;
        for (const auto &marker : markers) {
            float min_x = marker.at(0).x - rect.x;
            float max_x = marker.at(0).x - rect.x;

            for (size_t i = 0; i < marker.size(); ++i) {
                min_x = std::min(marker[i].x - rect.x, min_x);
                max_x = std::max(marker[i].x - rect.x, max_x);
            }

            extents.push_back(min_x);
            extents.push_back(max_x);
        }

        // Sort the extents in left to right order.
        std::sort(extents.begin(), extents.end(), [](float a, float b) {
            return a < b;
        });

        // Create an image the same size as the bounding box for the markers.
        cv::Mat image(rect.height, rect.width, CV_8UC1, cv::Scalar(0));

        // Represent marker corners using integer values (fillPoly requires integer values).
        std::vector<std::vector<cv::Point2i>> int_markers;
        for (auto marker : markers) {
            std::vector<cv::Point2i> points;

            for (const auto &p : marker)
                points.push_back(cv::Point2i(p.x - rect.x, p.y - rect.y));

            int_markers.push_back(points);
        }

        // Draw the markers in solid white.
        cv::fillPoly(image, int_markers, cv::Scalar(255));

        // Use marker extents to find the best line of symmetry.
        int best_score = 0;
        float best_line_x = 0;

        for (size_t i = 0; i + 1 < extents.size(); ++i) {
            const auto line_x = round((extents[i] + extents[i + 1]) / 2.0);
            const auto score = symmetry_score(image, line_x);

            if (score > best_score)
            {
                best_score = score;
                best_line_x = line_x;
            }
        }

        // Return the line of symmetry expressed in the original coordinate system.
        return best_line_x + rect.x;
    }

    /* Find the point of intersection between a vertical line and a line segment.
    */
    std::optional<cv::Point2f> vertical_line_intersection(float line_x, const cv::Point2f& p1, const cv::Point2f& p2) {
        float min_x = std::min(p1.x, p2.x);
        float max_x = std::max(p1.x, p2.x);

        if (min_x > line_x || max_x < line_x)
            return std::nullopt;

        const float delta_x = p2.x - p1.x;

        if (std::abs(delta_x) < 1e-5f)
            return cv::Point2f(line_x, (p1.y + p2.y) / 2.0f);

        const float t = (line_x - p1.x) / delta_x;
        const float y_intercept = p1.y + t * (p2.y - p1.y);

        return cv::Point2f(line_x, y_intercept);
    }

    /* Determine whether a vertical line intersects the given marker.
    */
    bool vertical_line_intersects_marker(const float line_x, const std::array<cv::Point2f, 4> &corners)
    {
        for (size_t i = 0; i < corners.size(); ++i) {
            const auto p0 = corners[i];
            const auto p1 = corners[(i + 1) % corners.size()];

            auto intersection = vertical_line_intersection(line_x, p0, p1);
            if (intersection.has_value())
                return true;
        }

        return false;
    }
}
