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
* FSUIPCServer		FSUIPCServer is the Class, that communicates headpose-data	*
*					to games, using the FSUIPC.dll.			         			*
********************************************************************************/
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_fsuipc.h"

/** constructor **/
FTNoIR_Protocol_FSUIPC::FTNoIR_Protocol_FSUIPC()
{
	loadSettings();
	ProgramName = "Microsoft FS2004";

	prevPosX = 0.0f;
	prevPosY = 0.0f;
	prevPosZ = 0.0f;
	prevRotX = 0.0f;
	prevRotY = 0.0f;
	prevRotZ = 0.0f;
}

/** destructor **/
FTNoIR_Protocol_FSUIPC::~FTNoIR_Protocol_FSUIPC()
{
	//
	// Free the DLL
	//
	FSUIPCLib.unload();
}

/** helper to Auto-destruct **/
void FTNoIR_Protocol_FSUIPC::Release()
{
    delete this;
}

void FTNoIR_Protocol_FSUIPC::Initialize()
{
	return;
}

//
// Scale the measured value to the Joystick values
//
int FTNoIR_Protocol_FSUIPC::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;
double local_x;
	
	local_x = x;
	if (local_x > max_x) {
		local_x = max_x;
	}
	if (local_x < min_x) {
		local_x = min_x;
	}
	y = ( 16383 * local_x ) / max_x;

	return (int) y;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol_FSUIPC::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FSUIPC" );
	LocationOfDLL = iniFile.value ( "LocationOfDLL", FSUIPC_FILENAME ).toString();
	qDebug() << "FSUIPCServer::loadSettings() says: Location of DLL = " << LocationOfDLL;
	iniFile.endGroup ();
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol_FSUIPC::sendHeadposeToGame( THeadPoseData *headpose ) {
DWORD result;
TFSState pitch;
TFSState yaw;
TFSState roll;
WORD FSZoom;

float virtPosX;
float virtPosY;
float virtPosZ;

float virtRotX;
float virtRotY;
float virtRotZ;

//	qDebug() << "FSUIPCServer::run() says: started!";

	virtRotX = -1.0f * headpose->pitch;				// degrees
	virtRotY = headpose->yaw;
	virtRotZ = headpose->roll;

	virtPosX = 0.0f;											// cm, X and Y are not working for FS2002/2004!
	virtPosY = 0.0f;
	virtPosZ = -1.0f * headpose->z;

	//
	// Init. the FSUIPC offsets (derived from Free-track...)
	//
	pitch.Control = 66503;
	yaw.Control = 66504;
	roll.Control = 66505;

	//
	// Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
	//
	if ((prevPosX != virtPosX) || (prevPosY != virtPosY) || (prevPosZ != virtPosZ) ||
		(prevRotX != virtRotX) || (prevRotY != virtRotY) || (prevRotZ != virtRotZ)) {
		//
		// Open the connection
		//
		FSUIPC_Open(SIM_ANY, &result);

		//
		// Check the FS-version
		//
		if  (((result == FSUIPC_ERR_OK) || (result == FSUIPC_ERR_OPEN)) && 
			 ((FSUIPC_FS_Version == SIM_FS2K2) || (FSUIPC_FS_Version == SIM_FS2K4))) {
//			qDebug() << "FSUIPCServer::run() says: FSUIPC opened succesfully";
			//
			// Write the 4! DOF-data to FS. Only rotations and zoom are possible.
			//
			pitch.Value = scale2AnalogLimits(virtRotX, -180, 180);
			FSUIPC_Write(0x3110, 8, &pitch, &result);

			yaw.Value = scale2AnalogLimits(virtRotY, -180, 180);
			FSUIPC_Write(0x3110, 8, &yaw, &result);

			roll.Value = scale2AnalogLimits(virtRotZ, -180, 180);
			FSUIPC_Write(0x3110, 8, &roll, &result);

			FSZoom = (WORD) (64/50) * virtPosZ + 64;
			FSUIPC_Write(0x832E, 2, &FSZoom, &result);

			//
			// Write the data, in one go!
			//
			FSUIPC_Process(&result);
			if (result == FSUIPC_ERR_SENDMSG) {
				FSUIPC_Close();							//timeout (1 second) so assume FS closed
			}
		}
	}

	prevPosX = virtPosX;
	prevPosY = virtPosY;
	prevPosZ = virtPosZ;
	prevRotX = virtRotX;
	prevRotY = virtRotY;
	prevRotZ = virtRotZ;
}

//
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol_FSUIPC::checkServerInstallationOK( HANDLE handle )
{   
	qDebug() << "checkServerInstallationOK says: Starting Function";

	//
	// Load the DLL.
	//
	//FSUIPCLib.setFileName( LocationOfDLL );
	//if (FSUIPCLib.load() != true) {
	//	qDebug() << "checkServerInstallationOK says: Error loading FSUIPC DLL";
	//	return false;
	//}
	//else {
	//	qDebug() << "checkServerInstallationOK says: FSUIPC DLL loaded.";
	//}

	return true;
}

//
// Return a name, if present the name from the Game, that is connected...
//
void FTNoIR_Protocol_FSUIPC::getNameFromGame( char *dest )
{   
	sprintf_s(dest, 99, "FS2002/2004");
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
	return new FTNoIR_Protocol_FSUIPC;
}

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FSUIPCControls::FSUIPCControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.btnFindDLL, SIGNAL(clicked()), this, SLOT(getLocationOfDLL()));

	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FSUIPCControls::~FSUIPCControls() {
	qDebug() << "~FSUIPCControls() says: started";
}

void FSUIPCControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void FSUIPCControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void FSUIPCControls::doOK() {
	save();
	this->close();
}

// override show event
void FSUIPCControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FSUIPCControls::doCancel() {
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
void FSUIPCControls::loadSettings() {

	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FSUIPC" );
	ui.txtLocationOfDLL->setText(iniFile.value ( "LocationOfDLL", FSUIPC_FILENAME ).toString() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FSUIPCControls::save() {

	qDebug() << "save() says: started";

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FSUIPC" );
	iniFile.setValue ( "LocationOfDLL", ui.txtLocationOfDLL->text() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//
// Show the Dialog to set the DLL's location
//
void FSUIPCControls::getLocationOfDLL()
{
	//
	// Get the new filename of the INI-file.
	//
	QString fileName = QFileDialog::getOpenFileName(this, tr("Locate file"),
													ui.txtLocationOfDLL->text(),
													tr("FSUIPC DLL file (FSUIPC*.dll);;All Files (*)"));
	if (!fileName.isEmpty()) {
		ui.txtLocationOfDLL->setText( fileName );
		settingsDirty = true;
	}
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol-settings dialog object.

// Export both decorated and undecorated names.
//   GetProtocolDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetProtocolDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetProtocolDialog=_GetProtocolDialog@0")

FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialogPtr __stdcall GetProtocolDialog( )
{
	return new FSUIPCControls;
}
