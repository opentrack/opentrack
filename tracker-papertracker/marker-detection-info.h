/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <opencv2/core.hpp>

namespace papertracker {
    struct marker_detection_info : public std::array<cv::Point2f, 4> {
        int id;
        bool solved;
        cv::Vec3d rvec;
        cv::Vec3d tvec;
        double z_angle;
        double weight;

        marker_detection_info(int id, const std::vector<cv::Point2f> &corners) : id(id), solved(false), z_angle(0), weight(1) {
            for (size_t i = 0; i < corners.size() && i < 4; ++i)
                (*this)[i] = corners[i];
        }
    };
}