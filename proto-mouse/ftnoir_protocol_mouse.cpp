/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_protocol_mouse.h"

#include "api/plugin-api.hpp"
#include "compat/math.hpp"

#include <cmath>
#include <algorithm>

#include <windows.h>

#ifndef MOUSEEVENTF_MOVE_NOCOALESCE
#   define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000
#endif

static const double invert[] = {
    1., 1., 1.,
    1., -1., 1.
};

void mouse::pose(const double* headpose, const double*)
{
    const int axis_x = s.mouse_x - 1;
    const int axis_y = s.mouse_y - 1;

    int mouse_x = 0, mouse_y = 0;

    if (axis_x == std::clamp(axis_x, (int)Axis_MIN, (int)Axis_MAX))
        mouse_x = get_value(headpose[axis_x] * invert[axis_x],
                            *s.sensitivity_x,
                            axis_x >= 3);

    if (axis_y == std::clamp(axis_y, (int)Axis_MIN, (int)Axis_MAX))
        mouse_y = get_value(headpose[axis_y] * invert[axis_y],
                            *s.sensitivity_y,
                            axis_y >= 3);

    int dx = get_delta(mouse_x, last_x), dy = mouse_y - last_y;

    last_x = mouse_x; last_y = mouse_y;

    if (dx || dy)
    {
        switch (s.input_method)
        {
        default:
            eval_once(qDebug() << "proto/mouse: invalid input method");
            [[fallthrough]];
        case input_direct:
        {
            INPUT input;
            input.type = INPUT_MOUSE;
            MOUSEINPUT& mi = input.mi;
            mi = {};
            mi.dx = dx;
            mi.dy = dy;
            mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_MOVE_NOCOALESCE;

            (void)SendInput(1, &input, sizeof(input));

            break;
        }
        case input_legacy:
        {
            POINT pt{};
            (void)GetCursorPos(&pt);
            (void)SetCursorPos(pt.x + dx, pt.y + dy);

            break;
        }
        }
    }
}

QString mouse::game_name()
{
    return tr("Mouse tracker");
}

int mouse::get_delta(int val, int prev)
{
    const int a = std::abs(val - prev), b = std::abs(val + prev);
    if (b < a)
        return val + prev;
    else
        return val - prev;
}

int mouse::get_value(double val, double sensitivity, bool is_rotation)
{
    constexpr double c[] = { 1e-3, 1e-1 };

    return iround(val * sensitivity * c[unsigned(is_rotation)]);
}

OPENTRACK_DECLARE_PROTOCOL(mouse, MOUSEControls, mouseDll)
