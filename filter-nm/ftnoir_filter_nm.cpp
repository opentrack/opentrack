/* Copyright (c) 2023 Tom Brazier <tom_github@firstsolo.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_nm.h"
#include "compat/math-imports.hpp"
#include "compat/macros.h"

#include "api/plugin-api.hpp"
#include "opentrack/defs.hpp"

#include <algorithm>

filter_nm::filter_nm()
{
}

void filter_nm::filter(const double* input, double* output)
{
    tVector position = { input[TX], input[TY], input[TZ] };
    tQuat rotation = QuatFromYPR(input + Yaw);

    // order of axes: x, y, z, yaw, pitch, roll
    if (first_run) [[unlikely]]
    {
        first_run = false;
        t.start();

        last_pos_speed = tVector();
        last_rot_speed = tQuat();
        last_pos_out = position;
        last_rot_out = rotation;
    }
    else
    {
        const double dt = t.elapsed_seconds();
        t.start();

        const tVector pos_speed = (position - last_pos_in) / dt;
        const double pos_tau = 1. / *s.pos_responsiveness;
        double alpha = dt / (dt + pos_tau);
        last_pos_speed += (pos_speed - last_pos_speed) * alpha;
        const double factor_pos = min(1.0, VectorLength(last_pos_speed) / (*s.pos_drift_speed * 3.0));
        alpha *= factor_pos * factor_pos;
        last_pos_out += (position - last_pos_out) * alpha;

        const tQuat rot_delta = QuatDivide(rotation, last_rot_in);
        constexpr double ms_per_s = 1000.0;             // angular speed quaternions need to be small to work so use °/ms
        const tQuat rot_speed = Slerp(tQuat(), rot_delta, 1.0 / ms_per_s / dt );
        const double rot_tau = 1. / *s.rot_responsiveness;
        alpha = dt / (dt + rot_tau);
        last_rot_speed = Slerp(last_rot_speed, rot_speed, alpha);
        const double angular_speed = AngleBetween(tQuat(), last_rot_speed) * ms_per_s;
        const double factor_rot = min(1.0, angular_speed / (*s.rot_drift_speed * 3.0));
        alpha *= factor_rot * factor_rot;
        last_rot_out = Slerp(last_rot_out, rotation, alpha);
    }

    last_pos_in = position;
    last_rot_in = rotation;
    std::copy(last_pos_out.v, last_pos_out.v + 3, output + TX);
    QuatToYPR(last_rot_out, &output[Yaw]);
}

OPENTRACK_DECLARE_FILTER(filter_nm, dialog_nm, nmDll)
