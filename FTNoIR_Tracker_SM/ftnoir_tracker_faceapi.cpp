#include "ftnoir_tracker_sm.h"
#include <QtGui>

FTNoIR_Tracker_SM::FTNoIR_Tracker_SM()
{
	//allocate memory for the parameters
	parameterValueAsFloat.clear();
	parameterRange.clear();

	// Add the parameters to the list
	parameterRange.append(std::pair<float,float>(1000.0f,9999.0f));
	parameterValueAsFloat.append(0.0f);
	setParameterValue(kPortAddress,5551.0f);
}

FTNoIR_Tracker_SM::~FTNoIR_Tracker_SM()
{
	qDebug() << "stopTracker says: terminating";

	if ( pMemData != NULL ) {
		UnmapViewOfFile ( pMemData );
	}
	
	CloseHandle( hSMMutex );
	CloseHandle( hSMMemMap );
	hSMMemMap = 0;
}

void FTNoIR_Tracker_SM::Release()
{
	qDebug() << "FTNoIR_Tracker_SM::Release says: Starting ";

	if ( pMemData != NULL ) {
		pMemData->command = FT_SM_EXIT;			// Issue 'exit' command
	}
	delete this;
}

void FTNoIR_Tracker_SM::Initialize( QFrame *videoframe )
{
	qDebug() << "FTNoIR_Tracker_SM::Initialize says: Starting ";
	loadSettings();

	if (SMCreateMapping()) {
		qDebug() << "FTNoIR_Tracker_SM::Initialize Mapping created.";
	}
	else {
		QMessageBox::warning(0,"FaceTrackNoIR Error","Memory mapping not created!",QMessageBox::Ok,QMessageBox::NoButton);
	}

	if ( pMemData != NULL ) {
		pMemData->command = 0;						// Reset any and all commands
		pMemData->handle = videoframe->winId();		// Handle of Videoframe widget
	}

	//
	// Start FTNoIR_FaceAPI_EXE.exe. The exe contains all faceAPI-stuff and is non-Qt...
	//
	QString program = "FTNoIR_FaceAPI_EXE.exe";
	faceAPI = new QProcess(0);
	faceAPI->start(program);

	// Show the video widget
	qDebug() << "FTNoIR_Tracker_SM::Initialize says: videoframe = " << videoframe;

	videoframe->show();
	return;
}

void FTNoIR_Tracker_SM::StartTracker( HWND parent_window )
{
	if ( pMemData != NULL ) {
		pMemData->command = FT_SM_START;				// Start command
	}
	return;
}

void FTNoIR_Tracker_SM::StopTracker( bool exit )
{

	qDebug() << "FTNoIR_Tracker_SM::StopTracker says: Starting ";
	// stops the faceapi engine
	if ( pMemData != NULL ) {
//		if (exit == true) {
			pMemData->command = (exit) ? FT_SM_EXIT : FT_SM_STOP;			// Issue 'stop' command
		//}
		//else {
		//	pMemData->command = FT_SM_STOP;				// Issue 'stop' command
		//}
	}
	return;
}

bool FTNoIR_Tracker_SM::GiveHeadPoseData(THeadPoseData *data)
{
	//
	// Check if the pointer is OK and wait for the Mutex.
	//
	if ( (pMemData != NULL) && (WaitForSingleObject(hSMMutex, 100) == WAIT_OBJECT_0) ) {

//		qDebug() << "FTNoIR_Tracker_SM::GiveHeadPoseData says: Retrieving data.";
		
		//
		// Copy the measurements to FaceTrackNoIR.
		//
		data->x     = pMemData->data.new_pose.head_pos.x * 100.0f;					// From meters to centimeters
		data->y     = pMemData->data.new_pose.head_pos.y * 100.0f;
		data->z     = pMemData->data.new_pose.head_pos.z * 100.0f;
		data->yaw   = pMemData->data.new_pose.head_rot.y_rads * 57.295781f;			// From rads to degrees
		data->pitch = pMemData->data.new_pose.head_rot.x_rads * 57.295781f;
		data->roll  = pMemData->data.new_pose.head_rot.z_rads * 57.295781f;
	
		ReleaseMutex(hSMMutex);
		return ( pMemData->data.new_pose.confidence > 0 );
	}
	return false;
}

bool FTNoIR_Tracker_SM::setParameterValue(const int index, const float newvalue)
{
	if ((index >= 0) && (index < parameterValueAsFloat.size()))
	{
		//
		// Limit the new value, using the defined range.
		//
		if (newvalue < parameterRange[index].first) {
			parameterValueAsFloat[index] = parameterRange[index].first;
		}
		else {
			if (newvalue > parameterRange[index].second) {
				parameterValueAsFloat[index] = parameterRange[index].second;
			}
			else {
				parameterValueAsFloat[index] = newvalue;
			}
		}

//		updateParameterString(index);
		return true;
	}
	else
	{
		return false;
	}
};

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker_SM::loadSettings() {

	qDebug() << "FTNoIR_Tracker_SM::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker_SM::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTNClient" );
	setParameterValue(kPortAddress, (float) iniFile.value ( "PortNumber", 5550 ).toInt());
	iniFile.endGroup ();
}

//
// Create a memory-mapping to the faceAPI data.
// It contains the tracking data, a command-code from FaceTrackNoIR
//
//
bool FTNoIR_Tracker_SM::SMCreateMapping()
{
	qDebug() << "FTNoIR_Tracker_SM::FTCreateMapping says: Starting Function";

	//
	// A FileMapping is used to create 'shared memory' between the faceAPI and FaceTrackNoIR.
	//
	// Try to create a FileMapping to the Shared Memory.
	// If one already exists: close it.
	//
	hSMMemMap = CreateFileMappingA( INVALID_HANDLE_VALUE , 00 , PAGE_READWRITE , 0 , 
		                           sizeof( TFaceData ) + sizeof( HANDLE ) + 100, 
								   (LPCSTR) SM_MM_DATA );

	if ( hSMMemMap != 0 ) {
		qDebug() << "FTNoIR_Tracker_SM::FTCreateMapping says: FileMapping Created!";
	}

	if ( ( hSMMemMap != 0 ) && ( (long) GetLastError == ERROR_ALREADY_EXISTS ) ) {
		CloseHandle( hSMMemMap );
		hSMMemMap = 0;
	}

	//
	// Create a new FileMapping, Read/Write access
	//
	hSMMemMap = OpenFileMappingA( FILE_MAP_ALL_ACCESS , false , (LPCSTR) SM_MM_DATA );
	if ( ( hSMMemMap != 0 ) ) {
		qDebug() << "FTNoIR_Tracker_SM::FTCreateMapping says: FileMapping Created again..." << hSMMemMap;
		pMemData = (SMMemMap *) MapViewOfFile(hSMMemMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(TFaceData));
		if (pMemData != NULL) {
			qDebug() << "FTNoIR_Tracker_SM::FTCreateMapping says: MapViewOfFile OK.";
//			pMemData->handle = handle;	// The game uses the handle, to send a message that the Program-Name was set!
		}
	    hSMMutex = CreateMutexA(NULL, false, SM_MUTEX);
	}
	else {
		qDebug() << "FTNoIR_Tracker_SM::FTCreateMapping says: Error creating Shared Memory for faceAPI!";
		return false;
	}

	//if (pMemData != NULL) {
	//	pMemData->data.DataID = 1;
	//	pMemData->data.CamWidth = 100;
	//	pMemData->data.CamHeight = 250;
	//}

	return true;
}



////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERHANDLE __stdcall GetTracker()
{
	return new FTNoIR_Tracker_SM;
}
