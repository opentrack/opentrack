/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_SETTINGS_H
#define FTNOIR_TRACKER_PT_SETTINGS_H

#include <opencv2/opencv.hpp>
#include "point_tracker.h"

#include "facetracknoir/options.h"
using namespace options;

struct settings
{
    pbundle b;
    value<int> cam_index,
               cam_res_x,
               cam_res_y,
               cam_fps,
               cam_roll,
               cam_pitch,
               cam_yaw,
               threshold,
               threshold_secondary,
               min_point_size,
               max_point_size;
    value<double> cam_f;

    value<int> m01_x, m01_y, m01_z;
    value<int> m02_x, m02_y, m02_z;
    value<bool> dyn_pose_res, video_widget;

    value<int> t_MH_x, t_MH_y, t_MH_z;

    value<int> reset_time;

    value<bool> bEnableYaw, bEnablePitch, bEnableRoll;
    value<bool> bEnableX, bEnableY, bEnableZ;

    value<int> clip_ty, clip_tz, clip_by, clip_bz;
    value<int> active_model_panel, cap_x, cap_y, cap_z;

    settings() :
        b(bundle("tracker-pt")),
        cam_index(b, "camera-index", 0),
        cam_res_x(b, "camera-res-width", 640),
        cam_res_y(b, "camera-res-height", 480),
        cam_fps(b, "camera-fps", 30),
        cam_roll(b, "camera-roll", 1),
        cam_pitch(b, "camera-pitch", 0),
        cam_yaw(b, "camera-yaw", 0),
        threshold(b, "threshold-primary", 128),
        threshold_secondary(b, "threshold-secondary", 128),
        min_point_size(b, "min-point-size", 10),
        max_point_size(b, "max-point-size", 50),
        cam_f(b, "camera-focal-length", 1),
        m01_x(b, "m_01-x", 0),
        m01_y(b, "m_01-y", 0),
        m01_z(b, "m_01-z", 0),
        m02_x(b, "m_02-x", 0),
        m02_y(b, "m_02-y", 0),
        m02_z(b, "m_02-z", 0),
        dyn_pose_res(b, "dynamic-pose-resolution", false),
        video_widget(b, "video-widget", true),
        t_MH_x(b, "model-centroid-x", 0),
        t_MH_y(b, "model-centroid-y", 0),
        t_MH_z(b, "model-centroid-z", 0),
        reset_time(b, "reset-time", 2000),
        bEnableYaw(b, "enable-yaw", true),
        bEnablePitch(b, "enable-pitch", true),
        bEnableRoll(b, "enable-roll", true),
        bEnableX(b, "enable-x", true),
        bEnableY(b, "enable-y", true),
        bEnableZ(b, "enable-z", true),
        clip_ty(b, "clip-ty", 0),
        clip_tz(b, "clip-tz", 0),
        clip_by(b, "clip-by", 0),
        clip_bz(b, "clip-bz", 0),
        active_model_panel(b, "active-model-panel", 0),
        cap_x(b, "cap-x", 0),
        cap_y(b, "cap-y", 0),
        cap_z(b, "cap-z", 0)
    {}
};

#endif //FTNOIR_TRACKER_PT_SETTINGS_H
