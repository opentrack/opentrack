/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
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
* PPJoyServer		PPJoyServer is the Class, that communicates headpose-data	*
*					to the Virtual Joystick, created by Deon van der Westhuysen.*
********************************************************************************/
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_ppjoy.h"

static const char* DevName = "\\\\.\\PPJoyIOCTL";

/** constructor **/
FTNoIR_Protocol::FTNoIR_Protocol()
{
char strDevName[100];

	// Initialize arrays
	for (int i = 0;i < 3;i++) {
		centerPos[i] = 0;
		centerRot[i] = 0;
	}
	selectedPPJoy = 1;
	loadSettings();

	/* Open a handle to the control device for the first virtual joystick. */
	/* Virtual joystick devices are named PPJoyIOCTL1 to PPJoyIOCTL16. */
	sprintf_s(strDevName, "%s%d", DevName, selectedPPJoy);
	h = CreateFileA((LPCSTR) strDevName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	/* Make sure we could open the device! */
	if (h == INVALID_HANDLE_VALUE)
	{
		QMessageBox::critical(0, "Connection Failed", QString("FaceTrackNoIR failed to connect to Virtual Joystick %1.\nCheck if it was properly installed!").arg(selectedPPJoy));
		return;
	}
}

/** destructor **/
FTNoIR_Protocol::~FTNoIR_Protocol()
{
	/* Make sure we could open the device! */
	if (h == INVALID_HANDLE_VALUE) {
		return;
	}

	//
	// Free the Virtual Joystick
	//
	CloseHandle(h);
}

/** helper to Auto-destruct **/
void FTNoIR_Protocol::Release()
{
    delete this;
}

void FTNoIR_Protocol::Initialize()
{
	return;
}

//
// Limit the Joystick values
//
void FTNoIR_Protocol::checkAnalogLimits() {
	for (int i = 0;i < NUM_ANALOG;i++) {
		if (Analog[i]>PPJOY_AXIS_MAX) {
			Analog[i]=PPJOY_AXIS_MAX;
		}
		else if (Analog[i]<PPJOY_AXIS_MIN) {
			Analog[i]=PPJOY_AXIS_MIN;
		}
	}
}

//
// Scale the measured value to the Joystick values
//
long FTNoIR_Protocol::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;

	y = ((PPJOY_AXIS_MAX - PPJOY_AXIS_MIN)/(max_x - min_x)) * x + ((PPJOY_AXIS_MAX - PPJOY_AXIS_MIN)/2) + PPJOY_AXIS_MIN;
//	qDebug() << "scale2AnalogLimits says: long_y =" << y;

	return (long) y;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "PPJoy" );
	selectedPPJoy = iniFile.value ( "Selection", 1 ).toInt();
	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol::sendHeadposeToGame( THeadPoseData *headpose, THeadPoseData *rawheadpose ) {
float virtPosX;
float virtPosY;
float virtPosZ;

float virtRotX;
float virtRotY;
float virtRotZ;

	//
	// Copy the Raw measurements.
	//
	virtRotX = headpose->pitch;
	virtRotY = headpose->yaw;
	virtRotZ = headpose->roll;

	virtPosX = headpose->x;
	virtPosY = headpose->y;
	virtPosZ = headpose->z;

	/* Initialise the IOCTL data structure */
	JoyState.Signature= JOYSTICK_STATE_V1;
	JoyState.NumAnalog= NUM_ANALOG;					// Number of analog values
	Analog= JoyState.Analog;						// Keep a pointer to the analog array for easy updating
	Digital= JoyState.Digital;						// Keep a pointer to the digital array for easy updating
	JoyState.NumDigital= NUM_DIGITAL;				// Number of digital values

	/* Make sure we could open the device! */
	/* MessageBox in run() does not work! (runtime error...)*/
	if (h == INVALID_HANDLE_VALUE) {
		return;
	}

	// The effective angle for faceTracking will be < 90 degrees, so we assume a smaller range here
	Analog[0] = scale2AnalogLimits( virtRotX, -50.0f, 50.0f );						// Pitch
	Analog[1] = scale2AnalogLimits( virtRotY, -50.0f, 50.0f );						// Yaw
	Analog[2] = scale2AnalogLimits( virtRotZ, -50.0f, 50.0f );						// Roll

	// The effective movement for faceTracking will be < 50 cm, so we assume a smaller range here
	Analog[3] = scale2AnalogLimits( virtPosX, -40.0f, 40.0f );						// X

	Analog[4] = scale2AnalogLimits( virtPosY, -40.0f, 40.0f );						// Y
	Analog[5] = scale2AnalogLimits( virtPosZ, -40.0f, 40.0f );						// Z

	checkAnalogLimits();

	/* Send request to PPJoy for processing. */
	/* Currently there is no Return Code from PPJoy, this may be added at a */
	/* later stage. So we pass a 0 byte output buffer.                      */
	if (!DeviceIoControl( h, IOCTL_PPORTJOY_SET_STATE, &JoyState, sizeof(JoyState), NULL, 0, &RetSize, NULL))
	{
		return;
	}
}

//
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol::checkServerInstallationOK( HANDLE handle )
{   
	/* Make sure we could open the device! */
	if (h == INVALID_HANDLE_VALUE) {
		return false;
	}
	return true;
}

//
// Return a name, if present the name from the Game, that is connected...
//
void FTNoIR_Protocol::getNameFromGame( char *dest )
{   
	sprintf_s(dest, 99, "Virtual PPJoy joystick");
	return;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocol     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetProtocol@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetProtocol=_GetProtocol@0")

FTNOIR_PROTOCOL_BASE_EXPORT IProtocolPtr __stdcall GetProtocol()
{
	return new FTNoIR_Protocol;
}
