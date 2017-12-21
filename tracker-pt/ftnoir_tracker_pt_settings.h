/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
using namespace options;

enum pt_color_type
{
    // explicit values, gotta preserve the numbering in .ini
    // don't reuse when removing some of the modes
    pt_color_natural = 2,
    pt_color_red_only = 3,
    pt_color_average = 5,
    pt_color_blue_only = 6,
};

struct settings_pt : opts
{
    value<QString> camera_name;
    value<int> cam_res_x,
               cam_res_y,
               cam_fps;
    value<double> min_point_size, max_point_size;

    value<int> m01_x, m01_y, m01_z;
    value<int> m02_x, m02_y, m02_z;

    value<int> t_MH_x, t_MH_y, t_MH_z;

    value<int> clip_ty, clip_tz, clip_by, clip_bz;
    value<int> active_model_panel, cap_x, cap_y, cap_z;

    value<int> fov;

    value<bool> dynamic_pose;
    value<int> init_phase_timeout;
    value<bool> auto_threshold;
    value<pt_color_type> blob_color;

    value<slider_value> threshold_slider;

    settings_pt() :
        opts("tracker-pt"),
        camera_name(b, "camera-name", ""),
        cam_res_x(b, "camera-res-width", 640),
        cam_res_y(b, "camera-res-height", 480),
        cam_fps(b, "camera-fps", 30),
        min_point_size(b, "min-point-size", 1),
        max_point_size(b, "max-point-size", 50),
        m01_x(b, "m_01-x", 0),
        m01_y(b, "m_01-y", 0),
        m01_z(b, "m_01-z", 0),
        m02_x(b, "m_02-x", 0),
        m02_y(b, "m_02-y", 0),
        m02_z(b, "m_02-z", 0),
        t_MH_x(b, "model-centroid-x", 0),
        t_MH_y(b, "model-centroid-y", 0),
        t_MH_z(b, "model-centroid-z", 0),
        clip_ty(b, "clip-ty", 40),
        clip_tz(b, "clip-tz", 30),
        clip_by(b, "clip-by", 70),
        clip_bz(b, "clip-bz", 80),
        active_model_panel(b, "active-model-panel", 0),
        cap_x(b, "cap-x", 40),
        cap_y(b, "cap-y", 60),
        cap_z(b, "cap-z", 100),
        fov(b, "camera-fov", 56),
        dynamic_pose(b, "dynamic-pose-resolution", true),
        init_phase_timeout(b, "init-phase-timeout", 250),
        auto_threshold(b, "automatic-threshold", true),
        blob_color(b, "blob-color", pt_color_natural),
        threshold_slider(b, "threshold-slider", slider_value(128, 0, 255))
    {
    }
};
