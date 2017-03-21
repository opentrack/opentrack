/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2017 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
#include <opencv2/core.hpp>

using namespace options;

struct settings_pt : opts
{
    enum model {
        clip_right, clip_left, cap,
    };

    value<int> threshold;
    value<double> min_point_size, max_point_size;

    value<int> t_MH_x, t_MH_y, t_MH_z;

    value<int> active_model_panel;

    settings_pt() :
        opts("tracker-pt"),
        threshold(b, "threshold-primary", 128),
        min_point_size(b, "min-point-size", 1),
        max_point_size(b, "max-point-size", 50),
        t_MH_x(b, "model-centroid-x", 0),
        t_MH_y(b, "model-centroid-y", 0),
        t_MH_z(b, "model-centroid-z", 0),
        active_model_panel(b, "active-model-panel", 0)
    {}

    cv::Vec3d get_model_offset()
    {
        cv::Vec3d offset(t_MH_x, t_MH_y, t_MH_z);
        if (offset[0] == 0 && offset[1] == 0 && offset[2] == 0)
        {
            int m = active_model_panel;
            switch (model(m))
            {
            default:
            // cap
            case cap: offset[0] = 0; offset[1] = 0; offset[2] = 0; break;
            // clip
            case clip_right: offset[0] = 135; offset[1] = 0; offset[2] = 0; break;
            // left clip
            case clip_left: offset[0] = -135; offset[1] = 0; offset[2] = 0; break;
            }
        }
        return offset;
    }
};
