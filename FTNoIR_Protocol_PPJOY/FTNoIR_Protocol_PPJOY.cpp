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
FTNoIR_Protocol_PPJOY::FTNoIR_Protocol_PPJOY()
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
FTNoIR_Protocol_PPJOY::~FTNoIR_Protocol_PPJOY()
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
void FTNoIR_Protocol_PPJOY::Release()
{
    delete this;
}

void FTNoIR_Protocol_PPJOY::Initialize()
{
	return;
}

//
// Limit the Joystick values
//
void FTNoIR_Protocol_PPJOY::checkAnalogLimits() {
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
long FTNoIR_Protocol_PPJOY::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;

	y = ((PPJOY_AXIS_MAX - PPJOY_AXIS_MIN)/(max_x - min_x)) * x + ((PPJOY_AXIS_MAX - PPJOY_AXIS_MIN)/2) + PPJOY_AXIS_MIN;
//	qDebug() << "scale2AnalogLimits says: long_y =" << y;

	return (long) y;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol_PPJOY::loadSettings() {
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
void FTNoIR_Protocol_PPJOY::sendHeadposeToGame( T6DOF *headpose ) {
float virtPosX;
float virtPosY;
float virtPosZ;

float virtRotX;
float virtRotY;
float virtRotZ;

	//
	// Copy the Raw measurements.
	//
	virtRotX = headpose->position.pitch;
	virtRotY = headpose->position.yaw;
	virtRotZ = headpose->position.roll;

	virtPosX = headpose->position.x;
	virtPosY = headpose->position.y;
	virtPosZ = headpose->position.z;

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
bool FTNoIR_Protocol_PPJOY::checkServerInstallationOK( HANDLE handle )
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
void FTNoIR_Protocol_PPJOY::getNameFromGame( char *dest )
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

FTNOIR_PROTOCOL_BASE_EXPORT PROTOCOLHANDLE __stdcall GetProtocol()
{
	return new FTNoIR_Protocol_PPJOY;
}

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
PPJOYControls::PPJOYControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.cbxSelectPPJoyNumber, SIGNAL(currentIndexChanged(int)), this, SLOT(virtualJoystickSelected( int )));

	for (int i = 1 ; i < 17; i++) {
		QString cbxText = QString("Virtual Joystick %1").arg(i);
		ui.cbxSelectPPJoyNumber->addItem(QIcon("images/PPJoy.ico"), cbxText);
	}
	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
PPJOYControls::~PPJOYControls() {
	qDebug() << "~PPJOYControls() says: started";
}

void PPJOYControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void PPJOYControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void PPJOYControls::doOK() {
	save();
	this->close();
}

// override show event
void PPJOYControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void PPJOYControls::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		qDebug() << "doCancel says: answer =" << ret;

		switch (ret) {
			case QMessageBox::Save:
				save();
				this->close();
				break;
			case QMessageBox::Discard:
				this->close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		this->close();
	}
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void PPJOYControls::loadSettings() {
	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "PPJoy" );
	ui.cbxSelectPPJoyNumber->setCurrentIndex(iniFile.value ( "Selection", 1 ).toInt() - 1);
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void PPJOYControls::save() {
	qDebug() << "save() says: started";

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "PPJoy" );
	iniFile.setValue ( "Selection", ui.cbxSelectPPJoyNumber->currentIndex() + 1 );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Handle changes of the Virtual Joystick selection
//
void PPJOYControls::virtualJoystickSelected( int index )
{
	settingsDirty = true;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol-settings dialog object.

// Export both decorated and undecorated names.
//   GetProtocolDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetProtocolDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetProtocolDialog=_GetProtocolDialog@0")

FTNOIR_PROTOCOL_BASE_EXPORT PROTOCOLDIALOGHANDLE __stdcall GetProtocolDialog( )
{
	return new PPJOYControls;
}
