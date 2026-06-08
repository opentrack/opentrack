/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "head.h"
#include "papertracker-util.h"
#include <opencv2/opencv.hpp>

namespace papertracker {
    void Head::set_handle_origin(const cv::Vec3d &origin)
    {
        handle_origin = origin;
    }

    bool Head::has_handle(int id) {
        return handles.count(id) > 0;
    }

    Marker &Head::get_handle(int id) {
        return handles.at(id);
    }

    void Head::set_handle(const Marker &handle_ref) {
        Marker handle = handle_ref;

        handle.tvec_local.set(handle.tvec_local.get() - handle_origin);

        handles[handle.id] = handle;
    }

    void Head::update_handle(int id, const cv::Vec3d &rvec, const cv::Vec3d &tvec) {
        handles.at(id).rvec_local.update(rvec);
        handles.at(id).tvec_local.update(tvec - handle_origin);
    }

    size_t Head::num_handles() {
        return handles.size();
    }

    std::pair<cv::Vec3d, cv::Vec3d> Head::get_pose_from_handle_transform(int id, cv::Vec3d &rvec_measured, cv::Vec3d &tvec_measured) {
        cv::Vec3d pose_rvec;
        cv::Vec3d pose_tvec;

        cv::Matx33d R_marker;
        cv::Rodrigues(rvec_measured, R_marker);

        cv::Matx33d R_handle;
        cv::Rodrigues(handles[id].rvec_local.get(), R_handle);

        cv::Matx33d R_pose = R_marker * R_handle.inv();
        cv::Rodrigues(R_pose, pose_rvec);

        const auto marker_tvec = handle_origin + handles[id].tvec_local.get();

        pose_tvec = R_pose * -marker_tvec + tvec_measured;

        return {pose_rvec, pose_tvec};
    }

    std::vector<int> Head::get_expected_visible_ids(double max_angle)
    {
        std::vector<int> visible;

        cv::Matx33d R_head;
        cv::Rodrigues(rvec, R_head);

        for (const auto &handle : handles) {
            cv::Matx33d R_handle;
            cv::Rodrigues(handle.second.rvec_local.get(), R_handle);

            cv::Matx33d R_composite = R_head * R_handle;

            cv::Vec3d marker_direction = tvec + R_head * (handle_origin + handle.second.tvec_local.get());
            marker_direction = marker_direction / -cv::norm(marker_direction);

            const double angle = get_marker_relative_angle(R_composite, marker_direction);

            if (angle <= max_angle)
                visible.push_back(handle.second.id);
        }

        return visible;
    }
}