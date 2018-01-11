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
    settings_pt() : opts("tracker-pt") {}

    value<QString> camera_name { b, "camera-name", "" };
    value<int> cam_res_x { b, "camera-res-width", 640 },
               cam_res_y { b, "camera-res-height", 480 },
               cam_fps { b, "camera-fps", 30 };
    value<double> min_point_size { b, "min-point-size", 2.5 },
                  max_point_size { b, "max-point-size", 50 };

    value<int> m01_x { b, "m_01-x", 0 }, m01_y { b, "m_01-y", 0 }, m01_z { b, "m_01-z", 0 };
    value<int> m02_x { b, "m_02-x", 0 }, m02_y { b, "m_02-y", 0 }, m02_z { b, "m_02-z", 0 };

    value<int> t_MH_x { b, "model-centroid-x", 0 },
               t_MH_y { b, "model-centroid-y", 0 },
               t_MH_z { b, "model-centroid-z", 0 };

    value<int> clip_ty { b, "clip-ty", 40 },
               clip_tz { b, "clip-tz", 30 },
               clip_by { b, "clip-by", 70 },
               clip_bz { b, "clip-bz", 80 };

    value<int> active_model_panel { b, "active-model-panel", 0 },
               cap_x { b, "cap-x", 40 },
               cap_y { b, "cap-y", 60 },
               cap_z { b, "cap-z", 100 };

    value<int> fov { b, "camera-fov", 56 };

    value<bool> dynamic_pose { b, "dynamic-pose-resolution", true };
    value<int> init_phase_timeout { b, "init-phase-timeout", 250 };
    value<bool> auto_threshold { b, "automatic-threshold", true };
    value<pt_color_type> blob_color { b, "blob-color", pt_color_natural };

    value<slider_value> threshold_slider { b, "threshold-slider", slider_value(128, 0, 255) };

#if 0

#define XSTR(x) #x

#define M(name, val) \
    { b, "interconnect-m" XSTR(name), (val) }
#define A(name) M(name, 0),
#define B(name) M(name, 1),

    value<double> interconnect[6][6] {
        { B(00) A(01) A(02) A(03) A(04) A(05) },
        { A(10) B(11) A(12) A(13) A(14) A(15) },
        { A(20) A(21) B(22) A(23) A(24) A(25) },
        { A(30) A(31) A(32) B(33) A(34) A(35) },
        { A(40) A(41) A(42) A(43) B(44) A(45) },
        { A(50) A(51) A(52) A(53) A(54) B(55) },
    };

#undef M
#undef A
#undef B
#undef XSTR

#endif
};
