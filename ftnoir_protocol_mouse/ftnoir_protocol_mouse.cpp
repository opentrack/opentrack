/* Copyright (c) 2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_protocol_mouse.h"
#include "opentrack/plugin-api.hpp"

#ifndef MOUSEEVENTF_MOVE_NOCOALESCE
#   define MOUSEEVENTF_MOVE_NOCOALESCE 0x2000
#endif

void FTNoIR_Protocol::pose(const double *headpose ) {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    if (hDesktop != NULL && GetWindowRect(hDesktop, &desktop)) {
        // XXX TODO remove axis selector, use mapping window's
        // axis selection. Mention in UI axis used. -sh 20140920
        int axis_x = s.Mouse_X;
        int axis_y = s.Mouse_Y;
        
        int mouse_x = 0, mouse_y = 0;
        
        if (axis_x > 0 && axis_x <= 6)
            mouse_x = headpose[axis_x-1] / (axis_x <= 3 ? 100 : 180) * 10 * desktop.right/2;
    
        if (axis_y > 0 && axis_y <= 6)
            mouse_y = headpose[axis_y-1] / (axis_y <= 3 ? 100 : 180) * 10 * desktop.bottom/2;
        
        MOUSEINPUT mi;
        mi.dx = mouse_x - last_x;
        mi.dy = mouse_y - last_y;
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
}

bool FTNoIR_Protocol::correct()
{   
    return true;
}

OPENTRACK_DECLARE_PROTOCOL(FTNoIR_Protocol, MOUSEControls, FTNoIR_ProtocolDll)
