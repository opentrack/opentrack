/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_protocol_mouse.h"
#include "api/plugin-api.hpp"
#include <cmath>
#include <algorithm>
#include <windows.h>

#ifndef MOUSEEVENTF_MOVE_NOCOALESCE
#   define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000
#endif

void mouse::pose(const double *headpose)
{
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
                            s.sensitivity_x(),
                            axis_x >= 3);
    }

    if (axis_y >= 0 && axis_y < 6)
        mouse_y = get_value(headpose[axis_y] * invert[axis_y],
                            s.sensitivity_y(),
                            axis_y >= 3);

    MOUSEINPUT mi;
    mi.dx = get_delta(mouse_x, last_x);
    mi.dy = get_delta(mouse_y, last_y);
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

QString mouse::game_name()
{
    return otr_tr("Mouse tracker");
}

int mouse::get_delta(int val, int prev)
{
    using std::abs;

    const int a = abs(val - prev), b = abs(val + prev);
    if (b < a)
        return val + prev;
    else
        return val - prev;
}

int mouse::get_value(double val, double sensitivity, bool is_rotation)
{
    static constexpr double sgn[] = { 1e-2, 1 };
    constexpr double c = 1e-1;

    return iround(val * c * sensitivity * sgn[unsigned(is_rotation)]);
}

mouse::mouse() : last_x(0), last_y(0) {}

OPENTRACK_DECLARE_PROTOCOL(mouse, MOUSEControls, mouseDll)
