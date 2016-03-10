/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_WIIMOTE_SETTINGS_H
#define FTNOIR_TRACKER_WIIMOTE_SETTINGS_H

#include "opentrack-compat/options.hpp"
using namespace options;

struct settings_wiimote : opts
{
    value<QString> camera_name;
    value<int> cam_res_x,
               cam_res_y;

    value<int> m01_x, m01_y, m01_z;
    value<int> m02_x, m02_y, m02_z;

    value<int> t_MH_x, t_MH_y, t_MH_z;

    value<int> clip_ty, clip_tz, clip_by, clip_bz;
    value<int> active_model_panel, cap_x, cap_y, cap_z;
    
    value<int> fov;
    
    value<bool> dynamic_pose;
    value<int> init_phase_timeout;

    settings_wiimote() :
        opts("tracker-wiimote"),
        camera_name(b, "camera-name", "wiimote"),
        cam_res_x(b, "camera-res-width", 1016),
        cam_res_y(b, "camera-res-height", 760),
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
        fov(b, "camera-fov", 42),
        dynamic_pose(b, "dynamic-pose-resolution", true),
        init_phase_timeout(b, "init-phase-timeout", 500)
    {}
};

#endif //FTNOIR_TRACKER_WIIMOTE_SETTINGS_H
