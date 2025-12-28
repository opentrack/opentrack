/* Copyright (c) 2012-2015 Stanislaw Halik
 * Copyright (c) 2023-2024 Michael Welter
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "accela_hamilton.h"
#include "api/plugin-api.hpp"
#include "opentrack/defs.hpp"

#include <algorithm>

#include "compat/math.hpp"
#include "compat/hamilton-tools.h"
#include "compat/math-imports.hpp"
#include "compat/time.hpp"


accela_hamilton::accela_hamilton()
{
    s.make_splines(spline_rot, spline_pos);
}


void accela_hamilton::filter(const double* input, double *output)
{
    constexpr double EPSILON = 1e-15F;

    const tQuat current_rot = QuatFromYPR(input + Yaw);
    const tVector current_pos(input[TX], input[TY], input[TZ]);

    if (first_run) [[unlikely]]
    {
        first_run = false;
        last_rotation = current_rot;
        last_position = current_pos;
        t.start();
#if defined DEBUG_ACCELA
        debug_max = 0;
        debug_timer.start();
#endif
        return;
    }

    const double pos_thres{s.pos_smoothing};
    const double pos_dz{ s.pos_deadzone};

    const double dt = t.elapsed_seconds();
    t.start();

    // Position
    {
        const tVector delta = current_pos - last_position;
        const double delta_len = VectorLength(delta);
        const tVector delta_normed = delta_len>0. ? delta/delta_len : tVector(); // Zero vector when length was zero.
        const double gain = dt*spline_pos.get_value_no_save(std::max(0., delta_len-pos_dz) / pos_thres);
        const tVector output_pos = last_position + (delta_normed * gain);
        output[TX] = output_pos.v[0];
        output[TY] = output_pos.v[1];
        output[TZ] = output_pos.v[2];
        last_position = output_pos;
    }

    // Zoom smoothing:
    const double zoomed_smoothing = [this](double output_z) {
        // Local copies because accessing settings involves thread synchronization
        // and I don't like this in the middle of math.
        const double max_zoomed_smoothing {s.max_zoomed_smoothing};		
        const double max_z {s.max_z};
        // Movement toward the monitor is negative. Negate and clamp it to get a positive value
        const double z = std::clamp(-output_z, 0., max_z);
        return max_zoomed_smoothing * z / (max_z + EPSILON);
    }(output[TZ]);

    const double rot_dz{ s.rot_deadzone};
    const double rot_thres = double{s.rot_smoothing} + zoomed_smoothing;

    // Rotation
    {
        // Inter/extrapolates along the arc between the old and new orientation.
        // It's basically a quaternion spherical linear interpolation, where the
        // accela gain x dt is the blending parameter. Might actually overshoot
        // the new orientation, but that's fine.

        // Compute rotation angle and axis which brings the previous orientation to the current rotation
        const double angle = AngleBetween(last_rotation, current_rot);
        // Apply the Accela gain magic. The "gain_angle" is the desired rotation from the old orientation
        // towards the current. Then alpha is the blending factor for the SLerp operation. It is normalized
        // to the range [0,1] where 1 corresponds to the current orientation, i.e. it is the fractional
        // rotation relative to the "gain_angle". EPSILON is added to prevent division by zero.
        // Additionally we use std::min(1., ...) to clamp the blending. alpha>1 would probably not make much
        // sense since it would mean extrapolation beyond the current orientation. And it would be a rare
        // edge case. Secondly idk if Slerp supports alpha>1.
        const double normalized_angle = std::max(0., angle - rot_dz) / rot_thres;
        const double gain_angle = dt*spline_rot.get_value_no_save(std::abs(normalized_angle));
        const double alpha = std::min(1., gain_angle / (angle + EPSILON));
        // Rotate toward the measured orientation. 
        const tQuat output_rot = Slerp(last_rotation, current_rot, alpha);
        // And back to Euler angles
        QuatToYPR(output_rot, output + Yaw);
        last_rotation = output_rot;
    }
}

namespace detail::accela_hamilton {

void settings_accela_hamilton::make_splines(spline& rot, spline& pos)
{
    rot.clear(); pos.clear();

    for (const auto& val : rot_gains)
        rot.add_point({ val.x, val.y });

    for (const auto& val : pos_gains)
        pos.add_point({ val.x, val.y });
}

} // ns detail::accela_hamilton

OPENTRACK_DECLARE_FILTER(accela_hamilton, dialog_accela_hamilton, accela_hamiltonDll)
