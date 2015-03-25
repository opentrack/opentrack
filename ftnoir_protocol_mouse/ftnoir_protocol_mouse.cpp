/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010-2011	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
* FTNoIR_Protocol_Mouse	The Class, that communicates headpose-data by			*
*						generating Mouse commands.								*
*						Many games (like FPS's) support Mouse-look features,	*
*						but no face-tracking.									*
********************************************************************************/
#include "ftnoir_protocol_mouse.h"
#include "opentrack/plugin-api.hpp"

void FTNoIR_Protocol::pose(const double *headpose ) {
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    if (hDesktop != NULL && GetWindowRect(hDesktop, &desktop)) {
        // XXX TODO remove axis selector, use mapping window's
        // axis selection. Mention in UI axis used. -sh 20140920
        int axis_x = s.Mouse_X;
        int axis_y = s.Mouse_Y;
        
        int mouse_x, mouse_y;
        
        if (axis_x > 0 && axis_x <= 6)
            mouse_x = headpose[axis_x-1] / (axis_x < 3 ? 100 : 180) * 10 * desktop.right/2;
    
        if (axis_y > 0 && axis_y <= 6)
            mouse_y = headpose[axis_y-1] / (axis_y < 3 ? 100 : 180) * 10 * desktop.bottom/2;
        
        POINT pt;
        
        if (GetCursorPos(&pt))
        {
            SetCursorPos(pt.x + mouse_x - last_x, pt.y + mouse_y - last_y);
            last_x = mouse_x;
            last_y = mouse_y;
        }
    }
}

bool FTNoIR_Protocol::correct()
{   
    return true;
}

extern "C" OPENTRACK_EXPORT IProtocol* GetConstructor()
{
    return new FTNoIR_Protocol;
}
