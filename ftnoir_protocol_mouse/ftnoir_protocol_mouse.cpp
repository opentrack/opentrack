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
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_mouse.h"
#include "facetracknoir/global-settings.h"

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
	prev_fMouse_X = 0.0f;
	prev_fMouse_Y = 0.0f;
	prev_fMouse_Wheel = 0.0f;
	frame_delay = 0;

	loadSettings();
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
}

/** helper to Auto-destruct **/
void FTNoIR_Protocol::Release()
{
    delete this;
}

void FTNoIR_Protocol::Initialize()
{
int ScreenX, ScreenY;

	ScreenX = GetSystemMetrics(SM_CXSCREEN);
	ScreenY = GetSystemMetrics(SM_CYSCREEN);

	qDebug() << "Initialize(): Screen width (x) = " << ScreenX << ", height (y) = " << ScreenY;

	return;
}

//
// Scale the measured value to the Joystick values
//
long FTNoIR_Protocol::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;

	y = ((MOUSE_AXIS_MAX - MOUSE_AXIS_MIN)/(max_x - min_x)) * x + ((MOUSE_AXIS_MAX - MOUSE_AXIS_MIN)/2) + MOUSE_AXIS_MIN;
	return (long) y;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Mouse" );
	Mouse_Style = (FTN_MouseStyle) (iniFile.value ( "Style", 1 ).toInt() - 1);
	Mouse_X = (FTN_AngleName) (iniFile.value ( "Mouse_X", 1 ).toInt() - 1);
	Mouse_Y = (FTN_AngleName) (iniFile.value ( "Mouse_Y", 1 ).toInt() - 1);
	Mouse_Wheel = (FTN_AngleName) (iniFile.value ( "Mouse_Wheel", 1 ).toInt() - 1);

	mouse_X_factor = iniFile.value("SensX", 10).toFloat() / 10.0f;
	mouse_Y_factor = iniFile.value("SensY", 10).toFloat() / 10.0f;
	mouse_Wheel_factor = iniFile.value("SensWheel", 10).toFloat() / 10.0f;

	useVirtualDesk = iniFile.value ( "useVirtualDesk", 0 ).toBool();

	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame(double *headpose, double *rawheadpose ) {
float fMouse_X = 0;							// The actual value
float fMouse_Y = 0;
float fMouse_Wheel = 0;


	//
	// Determine which of the 6DOF's is used.
	// The rotations are from -180 to +180 and the translations from -50cm to +50cm.
	// Let's scale the translations to the degrees for simplicity sake...
    //

    if (Mouse_X > 0 && Mouse_X < 6+1)
        fMouse_X = headpose[Mouse_X-1];

    if (Mouse_X < 3+1 && Mouse_X > 0)
        fMouse_X *= 3;

    if (Mouse_Y > 0 && Mouse_Y < 6+1)
        fMouse_Y = headpose[Mouse_Y-1];

    if (Mouse_Y < 3+1 && Mouse_Y > 0)
        fMouse_Y *= 3;

    if (Mouse_Wheel > 0 && Mouse_Wheel < 6+1)
        fMouse_Wheel = headpose[Mouse_Wheel-1];

    if (Mouse_Wheel < 3+1 && Mouse_Wheel > 0)
        fMouse_Wheel *= 3;

	//
	// Only send Input, when it has changed.
	// This releases the Mouse, when tracking is stopped (for a while).
	//
	if (frame_delay > 10) {
		if ((prev_fMouse_X != fMouse_X) || (prev_fMouse_Y != fMouse_Y) || (prev_fMouse_Wheel != fMouse_Wheel)) {
			SendInput(1, &MouseStruct, sizeof(MouseStruct));
		}

		prev_fMouse_X = fMouse_X;
		prev_fMouse_Y = fMouse_Y;
		prev_fMouse_Wheel = fMouse_Wheel;
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
