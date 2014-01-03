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
#include "facetracknoir/global-settings.h"

void FTNoIR_Protocol::sendHeadposeToGame(const double *headpose ) {
    double fMouse_X = 0;
    double fMouse_Y = 0;

    int Mouse_X = s.Mouse_X;
    int Mouse_Y = s.Mouse_Y;
	
    if (Mouse_X > 0 && Mouse_X <= 6)
        fMouse_X = headpose[Mouse_X-1] / (Mouse_X < 3 ? 100 : 180);

    if (Mouse_Y > 0 && Mouse_Y <= 6)
        fMouse_Y = headpose[Mouse_Y-1] / (Mouse_Y < 3 ? 100 : 180);
    
    RECT desktop;
    const HWND hDesktop = GetDesktopWindow();
    if (hDesktop != NULL && GetWindowRect(hDesktop, &desktop)) {
        fMouse_X *= desktop.right;
        fMouse_Y *= desktop.bottom;
        SetCursorPos(fMouse_X + desktop.right/2, fMouse_Y + desktop.bottom/2);
    }
}

void FTNoIR_Protocol::reload()
{
    s.b->reload();
}

bool FTNoIR_Protocol::checkServerInstallationOK()
{   
    return true;
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
