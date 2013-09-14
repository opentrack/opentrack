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

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	loadSettings();
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Mouse" );
	Mouse_X = (FTN_AngleName) (iniFile.value ( "Mouse_X", 0 ).toInt());
	Mouse_Y = (FTN_AngleName) (iniFile.value ( "Mouse_Y", 0 ).toInt());
	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame(double *headpose, double *rawheadpose ) {
    float fMouse_X = 0;
    float fMouse_Y = 0;
	
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

//
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK()
{   

	return true;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocol* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Protocol;
}
