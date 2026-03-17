/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "head.h"
#include "arucohead-util.h"
#include <opencv2/opencv.hpp>

namespace arucohead {
    bool Head::has_handle(int id) {
        return handles.count(id) > 0;
    }

    void Head::set_handle(const Marker &handle) {
        handles[handle.id] = handle;
    }

    Marker &Head::get_handle(int id) {
        return handles.at(id);
    }

    std::pair<cv::Vec3d, cv::Vec3d> Head::get_marker_local_transform(cv::Vec3d &rvec_measured, cv::Vec3d &tvec_measured, double xz_reference, double y_reference) {
        return arucohead::get_marker_local_transform(rvec_measured, tvec_measured, rvec, tvec, xz_reference, y_reference);
    }

    std::pair<cv::Vec3d, cv::Vec3d> Head::get_pose_from_handle_transform(int id, cv::Vec3d &rvec_measured, cv::Vec3d &tvec_measured, double xz_reference, double y_reference) {
        cv::Vec3d pose_rvec;
        cv::Vec3d pose_tvec;

        cv::Mat R_marker;
        cv::Rodrigues(rvec_measured, R_marker);

        cv::Mat R_handle;
        cv::Rodrigues(handles[id].rvec_local.get(), R_handle);

        cv::Mat R_pose = R_marker * R_handle.inv();
        cv::Rodrigues(R_pose, pose_rvec);

        auto expanded_tvec = expand_tvec(handles[id].tvec_local.get(), xz_reference, y_reference);
        cv::Mat pose_tvec_mat = R_pose * cv::Mat(-expanded_tvec) + cv::Mat(tvec_measured);
        pose_tvec = pose_tvec_mat.reshape(1, 1);

        return {pose_rvec, pose_tvec};
    }
}
