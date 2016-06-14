/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_protocol_mouse.h"
#include "opentrack/plugin-api.hpp"
#include <cmath>
#include <algorithm>
#include <windows.h>

#ifndef MOUSEEVENTF_MOVE_NOCOALESCE
#   define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000
#endif

void FTNoIR_Protocol::pose(const double *headpose)
{
    // XXX TODO remove axis selector, use mapping window's
    // axis selection. Mention in UI axis used. -sh 20140920
    const int axis_x = s.Mouse_X - 1;
    const int axis_y = s.Mouse_Y - 1;

    int mouse_x = 0, mouse_y = 0;

    static constexpr double invert[] =
    {
        1.,  1., 1.,
        1., -1., 1.
    };

    if (axis_x >= 0 && axis_x < 6)
    {
        mouse_x = get_value(headpose[axis_x] * invert[axis_x],
                            last_pos_x,
                            last_x,
                            axis_x >= 3,
                            static_cast<slider_value>(s.sensitivity_x));
    }

    if (axis_y >= 0 && axis_y < 6)
        mouse_y = get_value(headpose[axis_y] * invert[axis_y],
                            last_pos_y,
                            last_y,
                            axis_y >= 3,
                            static_cast<slider_value>(s.sensitivity_y));

    MOUSEINPUT mi;
    mi.dx = mouse_x;
    mi.dy = mouse_y;
    mi.mouseData = 0;
    mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;
    mi.time = 0;
    mi.dwExtraInfo = 0;
    INPUT input;
    input.type = INPUT_MOUSE;
    input.mi = mi;
    (void) SendInput(1, &input, sizeof(INPUT));

    last_x = mouse_x;
    last_y = mouse_y;
}

QString FTNoIR_Protocol::game_name()
{
    return "Mouse tracker";
}

double FTNoIR_Protocol::get_rotation(double val, double last_pos)
{
    using std::fmod;
    using std::fabs;
    using std::copysign;
    using std::min;

    const double x_1 = val - last_pos;

    if (x_1 > 180)
        return 360 - x_1;
    else if (x_1 < -180)
        return 360 + x_1;
    else
        return x_1;
}

int FTNoIR_Protocol::get_value(double val, double& last_pos, int& last_px, bool is_rotation, double sensitivity)
{
    static constexpr double c = 1e-1;

    if (is_rotation)
    {
        const double rot_delta = get_rotation(val, last_pos);

        last_pos = val;
        last_px = int(rot_delta * c * sensitivity);
    }
    else
    {
        val -= last_pos;

        last_pos = val;
        last_px = int(val * c * sensitivity / 100 - last_px);
    }

    return last_px;
}

bool FTNoIR_Protocol::correct()
{
    return true;
}

OPENTRACK_DECLARE_PROTOCOL(FTNoIR_Protocol, MOUSEControls, FTNoIR_ProtocolDll)
