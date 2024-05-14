/* Copyright (c) 2012-2015 Stanislaw Halik
 * Copyright (c) 2023-2024 Michael Welter
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "accela_hamilton.h"
#include "compat/math.hpp"
#include "api/plugin-api.hpp"
#include "opentrack/defs.hpp"

#include <algorithm>
#include <QDebug>
#include <QMutexLocker>

#include "compat/math-imports.hpp"
#include "compat/time.hpp"


accela_hamilton::accela_hamilton()
{
    s.make_splines(spline_rot, spline_pos);
}


void accela_hamilton::filter(const double* input, double *output)
{
    constexpr float EPSILON = 1e-15F;

    const QQuaternion current_rot = QQuaternion::fromEulerAngles(input[Pitch], input[Yaw], input[Roll]);
    const QVector3D current_pos(input[TX], input[TY], input[TZ]);

    if (unlikely(first_run))
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

    const float pos_thres{s.pos_smoothing};
    const float pos_dz{ s.pos_deadzone};

    const float dt = t.elapsed_seconds();
    t.start();

    // Position
    {
        const QVector3D delta = current_pos - last_position;
        const float delta_len = delta.length();
        QVector3D delta_normed = delta_len>0.F ? delta/delta_len : QVector3D(); // Zero vector when length was zero.
        const float gain = dt*spline_pos.get_value_no_save(std::max(0.F, delta_len-pos_dz) / pos_thres);
        const QVector3D output_pos = last_position + gain * delta_normed;
        output[TX] = output_pos.x();
        output[TY] = output_pos.y();
        output[TZ] = output_pos.z();
        last_position = output_pos;
    }

    // Zoom smoothing:
    const float zoomed_smoothing = [this](float output_z) {
        // Local copies because accessing settings involves thread synchronization
        // and I don't like this in the middle of math.
        const float max_zoomed_smoothing {s.max_zoomed_smoothing};		
        const float max_z {s.max_z};
        // Movement toward the monitor is negative. Negate and clamp it to get a positive value
        const float z = std::clamp(-output_z, 0.F, max_z);
        return max_zoomed_smoothing * z / (max_z + EPSILON);
    }(output[TZ]);

    const float rot_dz{ s.rot_deadzone};
    const float rot_thres = float{s.rot_smoothing} + zoomed_smoothing;

    // Rotation
    {
        // Inter/extrapolates along the arc between the old and new orientation.
        // It's basically a quaternion spherical linear interpolation, where the
        // accela gain x dt is the blending parameter. Might actually overshoot
        // the new orientation, but that's fine.

        // Compute rotation angle and axis which brings the previous orientation to the current rotation
        QVector3D axis;
        float angle;
        (last_rotation.conjugated() * current_rot).getAxisAndAngle(&axis, &angle);
        // Apply the Accela gain magic. Also need to multiply with dt here.
        angle = std::max(0.f, angle - std::copysign(rot_dz, angle)) / rot_thres;
        const float gain_angle = dt*spline_rot.get_value_no_save(std::abs(angle)) * signum(angle);
        // Rotate toward the measured orientation. We take the already computed axis. But the angle is now the accela gain.
        const QQuaternion output_rot = last_rotation * QQuaternion::fromAxisAndAngle(axis, gain_angle);
        // And back to Euler angles
        const QVector3D output_euler = output_rot.toEulerAngles();
        output[Pitch] =  output_euler.x();
        output[Yaw]   =  output_euler.y();
        output[Roll]  =  output_euler.z();
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
