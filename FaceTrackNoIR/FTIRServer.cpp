/********************************************************************************
* FTIRServer		FTIRServer is the Class, that communicates headpose-data	*
*					to games, using the NPClient.dll.		         			*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Testing and Research)						*
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
********************************************************************************/
/*
	Modifications (last one on top):
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
	20101127 - WVR: Added TrackIR.exe process for EZCA etc...
	20101023 - WVR: Added TIRViews for FS2004, Combat FS3, etc...
*/
#include "FTIRServer.h"

/** constructor **/
FTIRServer::FTIRServer() {
	loadSettings();
	ProgramName = "";
	dummyTrackIR = 0;
	viewsStart = 0;
	viewsStop = 0;
}

/** destructor **/
FTIRServer::~FTIRServer() {

	//
	// Destroy the File-mapping
	//
	FTIRDestroyMapping();

	//
	// Free the DLL's
	//
	FTIRClientLib.unload();
	if (viewsStop != NULL) {
		viewsStop();
		FTIRViewsLib.unload();
	}

	//
	// Kill the dummy TrackIR process.
	//
	qDebug() << "FTIRServer::~FTIRServer() about to kill TrackIR.exe process";
	try {
		if (dummyTrackIR) {
			dummyTrackIR->kill();
		}
	} 
	catch (...)
    {
		qDebug() << "~FTIRServer says: some error occurred";
	}

}

/** destructor **/
void FTIRServer::stopServer() {

	////
	//// Destroy the File-mapping
	////
	//FTIRDestroyMapping();

	////
	//// Free the DLL's
	////
	//try {
	//	FTIRClientLib.unload();
	//	if (useTIRViews && FTIRViewsLib.isLoaded()) {
	//		FTIRViewsLib.unload();
	//	}
	//}
	//catch (...)
	//{
	//	qDebug() << "~FTIRServer says: some error occurred";
	//}

	////
	//// Kill the dummy TrackIR process.
	////
	//qDebug() << "FTIRServer::~FTIRServer() about to kill TrackIR.exe process";
	//try {
	//	if (dummyTrackIR) {
	//		dummyTrackIR->kill();
	//	}
	//} 
	//catch (...)
 //   {
	//	qDebug() << "~FTIRServer says: some error occurred";
	//}

}

//
// Update Headpose in Game.
//
void FTIRServer::sendHeadposeToGame() {

	//
	// Check if the pointer is OK and wait for the Mutex.
	// Use the setposition in the (special) DLL, to write the headpose-data.
	//
	if ( (pMemData != NULL) && (WaitForSingleObject(hFTIRMutex, 100) == WAIT_OBJECT_0) ) {
		setposition (virtPosX, virtPosY, virtPosZ, virtRotZ, virtRotX, virtRotY );
		ReleaseMutex(hFTIRMutex);
	}
}

//
// Create a memory-mapping to the TrackIR data.
// It contains the tracking data, a handle to the main-window and the program-name of the Game!
//
//
bool FTIRServer::FTIRCreateMapping( HANDLE handle )
{
	qDebug() << "FTIRCreateMapping says: Starting Function";

	//
	// A FileMapping is used to create 'shared memory' between the FTIRServer and the FTClient.
	//
	// Try to create a FileMapping to the Shared Memory.
	// If one already exists: close it.
	//
	hFTIRMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 , 
		                           sizeof( TRACKIRDATA ) + sizeof( HANDLE ) + 100, 
								   (LPCSTR) FTIR_MM_DATA );

	if ( hFTIRMemMap != 0 ) {
		qDebug() << "FTIRCreateMapping says: FileMapping Created!" << hFTIRMemMap;
	}

	if ( ( hFTIRMemMap != 0 ) && ( (long) GetLastError == ERROR_ALREADY_EXISTS ) ) {
		CloseHandle( hFTIRMemMap );
		hFTIRMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
	//
	hFTIRMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) FTIR_MM_DATA );
	if ( ( hFTIRMemMap != 0 ) ) {
		qDebug() << "FTIRCreateMapping says: FileMapping Created again:" << hFTIRMemMap;
		pMemData = (FTIRMemMap *) MapViewOfFile(hFTIRMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TRACKIRDATA) + sizeof(hFTIRMemMap) + 100);
		if (pMemData != NULL) {
			pMemData->RegisteredHandle = handle;	// The game uses the handle, to send a message that the Program-Name was set!
		}
	    hFTIRMutex = CreateMutexA(NULL, false, FTIR_MUTEX);
	}
	else {
		QMessageBox::information(0, "FaceTrackNoIR error", QString("FTIRServer Error! \n"));
		return false;
	}

	return true;
}

//
// Destory the FileMapping to the shared memory
//
void FTIRServer::FTIRDestroyMapping()
{
	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}
	
	if (hFTIRMutex != 0) {
		CloseHandle( hFTIRMutex );
	}
	hFTIRMutex = 0;
	
	if (hFTIRMemMap != 0) {
		CloseHandle( hFTIRMemMap );
	}
	hFTIRMemMap = 0;

}

//
// Get the program-name from the client (Game!).
//
QString FTIRServer::GetProgramName() {   
QString *str;

	str = new QString("Test");
	return *str;
}

//
// Check if the Client DLL exists and load it (to test it), if so.
// Returns 'true' if all seems OK.
//
bool FTIRServer::checkServerInstallationOK( HANDLE handle )
{   
	QSettings settings("NaturalPoint", "NATURALPOINT\\NPClient Location");	// Registry settings (in HK_USER)
	QString aLocation;														// Location of Client DLL
	QString aFileName;														// File Path and Name

	//importProvider provider;
	//char *pProvider;

	qDebug() << "FTCheckClientDLL says: Starting Function";

	try {

		//
		// Load the NPClient.dll from the current path of FaceTrackNoIR, because there is no
		// guarantee TrackIR or GlovePIE is also installed.
		//
		// Write this path in the registry (under NaturalPoint/NATURALPOINT, for the game(s).
		//
		aLocation =  QCoreApplication::applicationDirPath() + "/";
		qDebug() << "FTCheckClientDLL says: Location of DLL =" << aLocation;

		//
		// Append a '/' to the Path and then the name of the dll.
		//
		aFileName = aLocation;
		aFileName.append(FTIR_CLIENT_FILENAME);
		qDebug() << "FTCheckClientDLL says: Full path of DLL =" << aFileName;
						
		if ( QFile::exists( aFileName ) ) {
			qDebug() << "FTCheckClientDLL says: DLL exists!";
			//
			// Write the path to the key in the Registry, so the game(s) can find it too...
			//
			settings.setValue( "Path" , aLocation );

			//
			// Load the DLL and map to the functions in it.
			//
			FTIRClientLib.setFileName(aFileName);
			FTIRClientLib.load();
		}
		else {
			QMessageBox::information(0, "FaceTrackNoIR error", QString("Necessary file (NPClient.dll) was NOT found!\n"));
			return false;
		}

		//
		// Also load TIRViews.dll, to support some older games
		//
		if (useTIRViews) {
			aFileName = aLocation;
			aFileName.append(FTIR_VIEWS_FILENAME);
			FTIRViewsLib.setFileName(aFileName);
			FTIRViewsLib.load();
		}

		//
		// Start TrackIR.exe, also to support some older games and EZCA
		// Some TrackIR clients check if a process called TrackIR.exe is running.
		// This should do the trick
		//
		QString program = "TrackIR.exe";
 		dummyTrackIR = new QProcess(this);
		dummyTrackIR->start(program);

	} catch(...) {
		settings.~QSettings();
	}
	
	//
	// Create the File-mapping for Inter Process Communication
	//
	if (!FTIRCreateMapping( handle )){
		return false;
	}

	//
	// Find the functions in the DLL's
	//
	// Get the setposition function from the DLL and use it!
	//
	setposition = (importSetPosition) FTIRClientLib.resolve("SetPosition");
	if (setposition == NULL) {
		qDebug() << "FTIRServer::run() says: SetPosition function not found in DLL!";
		return false;
	}
	else {
		setposition (7.0f, 1.0f, 2.0f, 3.0f, 4.0f, 5.0f);
	}

	//
	// Load the Start function from TIRViews.dll and call it, to start compatibility with older games
	//
	if (useTIRViews) {
		viewsStart = (importTIRViewsStart) FTIRViewsLib.resolve("TIRViewsStart");
		if (viewsStart == NULL) {
			qDebug() << "FTIRServer::run() says: TIRViewsStart function not found in DLL!";
		}
		else {
			qDebug() << "FTIRServer::run() says: TIRViewsStart executed!";
			viewsStart();
		}

		//
		// Load the Stop function from TIRViews.dll. Call it when terminating the thread.
		//
		viewsStop = (importTIRViewsStop) FTIRViewsLib.resolve("TIRViewsStop");
		if (viewsStop == NULL) {
			qDebug() << "FTIRServer::run() says: TIRViewsStop function not found in DLL!";
		}
	}
	return true;
}

//
// Scale the measured value to the Joystick values
//
float FTIRServer::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;
double local_x;
	
	local_x = x;
	if (local_x > max_x) {
		local_x = max_x;
	}
	if (local_x < min_x) {
		local_x = min_x;
	}
	y = ( NP_AXIS_MAX * local_x ) / max_x;

	return (float) y;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTIRServer::loadSettings() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FTIR" );
	useTIRViews	= iniFile.value ( "useTIRViews", 0 ).toBool();
	iniFile.endGroup ();
}

//
// Constructor for server-settings-dialog
//
FTIRControls::FTIRControls( QWidget *parent, Qt::WindowFlags f ) :
QWidget( parent , f)
{
	QString aFileName;														// File Path and Name

	ui.setupUi( this );

	QPoint offsetpos(100, 100);
	this->move(parent->pos() + offsetpos);

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.chkTIRViews, SIGNAL(stateChanged(int)), this, SLOT(chkTIRViewsChanged()));

	aFileName = QCoreApplication::applicationDirPath() + "/";
	aFileName.append(FTIR_VIEWS_FILENAME);
	if ( !QFile::exists( aFileName ) ) {
		ui.chkTIRViews->setChecked( false );
		ui.chkTIRViews->setEnabled ( false );
	}
	else {
		ui.chkTIRViews->setEnabled ( true );
	}
	
	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
FTIRControls::~FTIRControls() {
	qDebug() << "~FTIRControls() says: started";
}

//
// OK clicked on server-dialog
//
void FTIRControls::doOK() {
	save();
	this->close();
}

// override show event
void FTIRControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void FTIRControls::doCancel() {
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
void FTIRControls::loadSettings() {

	qDebug() << "loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTIR" );
	ui.chkTIRViews->setChecked (iniFile.value ( "useTIRViews", 0 ).toBool());
	iniFile.endGroup ();

	settingsDirty = false;

}

//
// Save the current Settings to the currently 'active' INI-file.
//
void FTIRControls::save() {

	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "FTIR" );
	iniFile.setValue ( "useTIRViews", ui.chkTIRViews->isChecked() );
	iniFile.endGroup ();

	settingsDirty = false;
}

//END
