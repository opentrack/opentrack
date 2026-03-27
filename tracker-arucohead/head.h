/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include "marker.h"

namespace arucohead {
    class Head {
    private:
        std::unordered_map<int, Marker> handles;

    public:
        cv::Vec3d rvec;
        cv::Vec3d tvec;
        bool has_handle(int id);
        void set_handle(const Marker &handle);
        Marker &get_handle(int id);
        std::pair<cv::Vec3d, cv::Vec3d> get_marker_local_transform(cv::Vec3d &rvec, cv::Vec3d &tvec, double xz_reference, double y_reference);
        std::pair<cv::Vec3d, cv::Vec3d> get_pose_from_handle_transform(int id, cv::Vec3d &rvec, cv::Vec3d &tvec, double xz_reference, double y_reference);
    };
}