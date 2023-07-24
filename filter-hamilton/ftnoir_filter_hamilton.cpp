/* Copyright (c) 2020, GO63-samara <go1@list.ru> 
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_filter_hamilton.h"
#include <cmath>
#include <QMutexLocker>
#include "api/plugin-api.hpp"
#include "compat/hamilton-tools.h"

hamilton::hamilton() = default;

void hamilton::filter(const double *input, double *output)
{
    tQuat quat_input = QuatFromYPR( &input[Yaw] );

    if (first_run)
    {
        first_run = false;
        quat_last = quat_input;
        pos_last  = {input[TX], input[TY], input[TZ]};
        for (int i=0; i<6; i++) output[i] = input[i];
        return;
    }

    // positions:
    const double pos_max     {s.kMaxDist};
    const double pos_deadzone{s.kDeadZoneDist};
    const double pos_pow     {s.kPowDist};

    double dist	 = VectorDistance( &input[TX], pos_last);

    double alpha = (dist - pos_deadzone) / (pos_max + pos_deadzone + EPSILON);
    alpha = std::min(1.0, std::max(0.0, alpha));
    if (alpha > 0.0)
        alpha = pow(alpha, pos_pow);
    // Scale alpha so that alpha * dist <= dist - pos_deadzone. This ensures that
    // the center of the deadzone will never move closer to the input position than
    // distance dist. And this ensures that the view never jumps ahead of head
    // movements.
    alpha *= (dist - pos_deadzone) / (dist + EPSILON);

    pos_last = Lerp(pos_last, input, alpha);

    output[TX] = pos_last.v[0]; 
    output[TY] = pos_last.v[1]; 
    output[TZ] = pos_last.v[2]; 

    // zoom smoothing:
    const double pow_zoom {s.kPowZoom};		
    const double max_z    {s.kMaxZ};
    double rot_zoom = pow_zoom;

    if (output[TZ] > 0) rot_zoom = 0;
    else rot_zoom *= -output[TZ] / (max_z + EPSILON);
    rot_zoom = fmin( rot_zoom, pow_zoom );

    // rotations:
    const double rot_max     {s.kMaxRot};
    const double rot_pow     {s.kPowRot};
    const double rot_deadzone{s.kDeadZoneRot};
    
    double angle = AngleBetween(quat_input, quat_last);

    alpha = (angle - rot_deadzone) / (rot_max + rot_deadzone + EPSILON);
    alpha = std::min(1.0, std::max(0.0, alpha));
    if (alpha > 0.0)
        alpha = pow(alpha, rot_pow + rot_zoom);
    // see comment in earlier alpha calculation above
    alpha *= (angle - rot_deadzone) / (angle + EPSILON);

    quat_last = Slerp(quat_last, quat_input, alpha);

    QuatToYPR(quat_last, &output[Yaw] );
}

OPENTRACK_DECLARE_FILTER(hamilton, dialog_hamilton, hamiltonDll)
