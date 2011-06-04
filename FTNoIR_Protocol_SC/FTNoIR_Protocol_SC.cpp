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
* FTNoIR_Protocol_SC		FTNoIR_Protocol_SC is the Class, that communicates headpose-data			*
*				to games, using the SimConnect.dll.		         				*
*				SimConnect.dll is a so called 'side-by-side' assembly, so it	*
*				must be treated as such...										*
********************************************************************************/
/*
	Modifications (last one on top):
	20110401 - WVR: Moved protocol to a DLL, convenient for installation etc.
	20101224 - WVR: Base class is no longer inheriting QThread. sendHeadposeToGame
					is called from run() of Tracker.cpp
*/
#include "ftnoir_protocol_sc.h"

importSimConnect_CameraSetRelative6DOF FTNoIR_Protocol_SC::simconnect_set6DOF;
HANDLE FTNoIR_Protocol_SC::hSimConnect = 0;			// Handle to SimConnect

float FTNoIR_Protocol_SC::virtSCPosX = 0.0f;			// Headpose
float FTNoIR_Protocol_SC::virtSCPosY = 0.0f;
float FTNoIR_Protocol_SC::virtSCPosZ = 0.0f;
	
float FTNoIR_Protocol_SC::virtSCRotX = 0.0f;
float FTNoIR_Protocol_SC::virtSCRotY = 0.0f;
float FTNoIR_Protocol_SC::virtSCRotZ = 0.0f;

float FTNoIR_Protocol_SC::prevSCPosX = 0.0f;			// previous Headpose
float FTNoIR_Protocol_SC::prevSCPosY = 0.0f;
float FTNoIR_Protocol_SC::prevSCPosZ = 0.0f;
	
float FTNoIR_Protocol_SC::prevSCRotX = 0.0f;
float FTNoIR_Protocol_SC::prevSCRotY = 0.0f;
float FTNoIR_Protocol_SC::prevSCRotZ = 0.0f;

/** constructor **/
FTNoIR_Protocol_SC::FTNoIR_Protocol_SC()
{
	ProgramName = "Microsoft FSX";
	blnSimConnectActive = false;
	hSimConnect = 0;
}

/** destructor **/
FTNoIR_Protocol_SC::~FTNoIR_Protocol_SC()
{
	qDebug() << "~FTNoIR_Protocol_SC says: inside" << FTNoIR_Protocol_SC::hSimConnect;

	if (hSimConnect != 0) {
		qDebug() << "~FTNoIR_Protocol_SC says: before simconnect_close";
		if (SUCCEEDED( simconnect_close( FTNoIR_Protocol_SC::hSimConnect ) ) ) {
			qDebug() << "~FTNoIR_Protocol_SC says: close SUCCEEDED";
		}
	}
//	SCClientLib.unload(); Generates crash when tracker is ended...
}

/** helper to Auto-destruct **/
void FTNoIR_Protocol_SC::Release()
{
    delete this;
}

void FTNoIR_Protocol_SC::Initialize()
{
	return;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Protocol_SC::loadSettings() {
// None yet...
}

//
// Update Headpose in Game.
//
void FTNoIR_Protocol_SC::sendHeadposeToGame( T6DOF *headpose ) {


	virtSCRotX = -1.0f * headpose->position.pitch;					// degrees
	virtSCRotY = -1.0f * headpose->position.yaw;
	virtSCRotZ = headpose->position.roll;

	virtSCPosX = headpose->position.x/100.f;						// cm to meters
	virtSCPosY = headpose->position.y/100.f;
	virtSCPosZ = -1.0f * headpose->position.z/100.f;

	//
	// It's only useful to send data, if the connection was made.
	//
	if (!blnSimConnectActive) {
		if (SUCCEEDED(simconnect_open(&hSimConnect, "FaceTrackNoIR", NULL, 0, 0, 0))) {
			qDebug() << "FTNoIR_Protocol_SC::sendHeadposeToGame() says: SimConnect active!";

			//set up the events we want to listen for
			HRESULT hr;

			simconnect_subscribetosystemevent(hSimConnect, EVENT_PING, "Frame"); 

			hr = simconnect_mapclienteventtosimevent(hSimConnect, EVENT_INIT, "");
			hr = simconnect_addclienteventtonotificationgroup(hSimConnect, GROUP0, EVENT_INIT, false);
			hr = simconnect_setnotificationgrouppriority(hSimConnect, GROUP0, SIMCONNECT_GROUP_PRIORITY_HIGHEST);
			////hr = SimConnect_MapInputEventToClientEvent(hSimConnect, INPUT0, "VK_COMMA", EVENT_INIT);
			////hr = SimConnect_SetInputGroupState(hSimConnect, INPUT0, SIMCONNECT_STATE_ON);

			blnSimConnectActive = true;
		}
	}
	else {
		//
		// Write the 6DOF-data to FSX
//		//
//		// Only do this when the data has changed. This way, the HAT-switch can be used when tracking is OFF.
//		//
//		if ((prevPosX != virtPosX) || (prevPosY != virtPosY) || (prevPosZ != virtPosZ) ||
//			(prevRotX != virtRotX) || (prevRotY != virtRotY) || (prevRotZ != virtRotZ)) {
////			if (S_OK == simconnect_set6DOF(hSimConnect, virtPosX, virtPosY, virtPosZ, virtRotX, virtRotZ, virtRotY)) {
////					qDebug() << "FTNoIR_Protocol_SC::run() says: SimConnect data written!";
////			}
//		}
//
//		prevPosX = virtPosX;
//		prevPosY = virtPosY;
//		prevPosZ = virtPosZ;
//		prevRotX = virtRotX;
//		prevRotY = virtRotY;
//		prevRotZ = virtRotZ;

		if SUCCEEDED(simconnect_calldispatch(hSimConnect, processNextSimconnectEvent, NULL)) {
			qDebug() << "FTNoIR_Protocol_SC::sendHeadposeToGame() says: Dispatching";
		}
		else {
			qDebug() << "FTNoIR_Protocol_SC::sendHeadposeToGame() says: Error Dispatching!";
		}
	}
}

//
// Returns 'true' if all seems OK.
//
bool FTNoIR_Protocol_SC::checkServerInstallationOK( HANDLE handle )
{   
	QString aFileName;														// File Path and Name

	// Code to activate the context for the SimConnect DLL
	ACTCTX act = { 0 };
	HANDLE hctx;
	ULONG_PTR ulCookie;


	qDebug() << "SCCheckClientDLL says: Starting Function";

	try {

		act.cbSize = sizeof(act);
		act.dwFlags = ACTCTX_FLAG_RESOURCE_NAME_VALID;

		QString manifest(QCoreApplication::applicationDirPath());
//		manifest += "\\FaceTrackNoIR.exe";
		manifest += "\\FTNoIR_Protocol_SC.dll";
		const wchar_t * encodedName = reinterpret_cast<const wchar_t *>(manifest.utf16());
		
		act.lpSource = encodedName;
		act.lpResourceName = MAKEINTRESOURCE(101);

		hctx = CreateActCtx (&act);

		if (hctx != INVALID_HANDLE_VALUE) { 
			if (!ActivateActCtx(hctx, &ulCookie)) { 
				ReleaseActCtx(hctx);
				qDebug() << "SCCheckClientDLL says: Error activating SimConnect manifest";
			}
		}
		else {
			qDebug() << "SCCheckClientDLL says: Error INVALID_HANDLE: " << GetLastError();
			return false;
		}

		//
		// Just try to load the DLL. Let Windows handle the PATH's and such trivialities...
		//
		aFileName = SC_CLIENT_FILENAME;
						
		//
		// Load the DLL.
		//
		SCClientLib.setFileName(aFileName);
		if (SCClientLib.load() != true) {
			qDebug() << "SCCheckClientDLL says: Error loading SimConnect DLL";
			return false;
		}

		//
		// Deactivate the context again: the function-references should stay in-tact...
		//
		DeactivateActCtx(0, ulCookie);
		ReleaseActCtx(hctx);

	} catch(...) {
		qDebug() << "SCCheckClientDLL says: Error loading SimConnect DLL";
		return false;
	}

	//
	// Get the functions from the DLL.
	//
	simconnect_open = (importSimConnect_Open) SCClientLib.resolve("SimConnect_Open");
	if (simconnect_open == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_Open function not found in DLL!";
		return false;
	}
	simconnect_set6DOF = (importSimConnect_CameraSetRelative6DOF) SCClientLib.resolve("SimConnect_CameraSetRelative6DOF");
	if (simconnect_set6DOF == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_CameraSetRelative6DOF function not found in DLL!";
		return false;
	}
	simconnect_close = (importSimConnect_Close) SCClientLib.resolve("SimConnect_Close");
	if (simconnect_close == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_Close function not found in DLL!";
		return false;
	}

	//return true;

	simconnect_calldispatch = (importSimConnect_CallDispatch) SCClientLib.resolve("SimConnect_CallDispatch");
	if (simconnect_calldispatch == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_CallDispatch function not found in DLL!";
		return false;
	}

	simconnect_subscribetosystemevent = (importSimConnect_SubscribeToSystemEvent) SCClientLib.resolve("SimConnect_SubscribeToSystemEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_SubscribeToSystemEvent function not found in DLL!";
		return false;
	}

	simconnect_mapclienteventtosimevent = (importSimConnect_MapClientEventToSimEvent) SCClientLib.resolve("SimConnect_MapClientEventToSimEvent");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_MapClientEventToSimEvent function not found in DLL!";
		return false;
	}

	simconnect_addclienteventtonotificationgroup = (importSimConnect_AddClientEventToNotificationGroup) SCClientLib.resolve("SimConnect_AddClientEventToNotificationGroup");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_AddClientEventToNotificationGroup function not found in DLL!";
		return false;
	}

	simconnect_setnotificationgrouppriority = (importSimConnect_SetNotificationGroupPriority) SCClientLib.resolve("SimConnect_SetNotificationGroupPriority");
	if (simconnect_subscribetosystemevent == NULL) {
		qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect_SetNotificationGroupPriority function not found in DLL!";
		return false;
	}

	qDebug() << "FTNoIR_Protocol_SC::checkServerInstallationOK() says: SimConnect functions resolved in DLL!";

	return true;
}

void CALLBACK FTNoIR_Protocol_SC::processNextSimconnectEvent(SIMCONNECT_RECV* pData, DWORD cbData, void *pContext)
{
//    HRESULT hr;

    switch(pData->dwID)
    {
        case SIMCONNECT_RECV_ID_EVENT:
        {
            SIMCONNECT_RECV_EVENT *evt = (SIMCONNECT_RECV_EVENT*)pData;

			qDebug() << "FTNoIR_Protocol_SC::processNextSimconnectEvent() says: SimConnect active!";
            //switch(evt->uEventID)
            //{
            //    //case EVENT_CAMERA_RIGHT:

            //    //    cameraBank = normalize180( cameraBank + 5.0f);

            //    //    hr = SimConnect_CameraSetRelative6DOF(hSimConnect, 0.0f, 0.0f, 0.0f,
            //    //            SIMCONNECT_CAMERA_IGNORE_FIELD,SIMCONNECT_CAMERA_IGNORE_FIELD, cameraBank);

            //    //    printf("\nCamera Bank = %f", cameraBank);
            //    //    break;

            //    //case EVENT_CAMERA_LEFT:
            //    //    
            //    //    cameraBank = normalize180( cameraBank - 5.0f);

            //    //    hr = SimConnect_CameraSetRelative6DOF(hSimConnect, 0.0f, 0.0f, 0.0f,
            //    //            SIMCONNECT_CAMERA_IGNORE_FIELD,SIMCONNECT_CAMERA_IGNORE_FIELD, cameraBank);
            //    //    
            //    //    printf("\nCamera Bank = %f", cameraBank);
            //    //    break;

            //    //default:
            //    //    break;
            //}
            //break;
        }
		case SIMCONNECT_RECV_ID_EVENT_FRAME:
		{
//			qDebug() << "FTNoIR_Protocol_SC::processNextSimconnectEvent() says: Frame event!";
			if ((prevSCPosX != virtSCPosX) || (prevSCPosY != virtSCPosY) || (prevSCPosZ != virtSCPosZ) ||
				(prevSCRotX != virtSCRotX) || (prevSCRotY != virtSCRotY) || (prevSCRotZ != virtSCRotZ)) {
				if (S_OK == simconnect_set6DOF(hSimConnect, virtSCPosX, virtSCPosY, virtSCPosZ, virtSCRotX, virtSCRotZ, virtSCRotY)) {
	//					qDebug() << "FTNoIR_Protocol_SC::run() says: SimConnect data written!";
				}
			}
			prevSCPosX = virtSCPosX;
			prevSCPosY = virtSCPosY;
			prevSCPosZ = virtSCPosZ;
			prevSCRotX = virtSCRotX;
			prevSCRotY = virtSCRotY;
			prevSCRotZ = virtSCRotZ;
		}

        case SIMCONNECT_RECV_ID_EXCEPTION:
        {
            SIMCONNECT_RECV_EXCEPTION *except = (SIMCONNECT_RECV_EXCEPTION*)pData;
            
            switch (except->dwException)
            {
            case SIMCONNECT_EXCEPTION_ERROR:
                printf("\nCamera error");
                break;

            default:
                printf("\nException");
                break;
            }
            break;
        }

        case SIMCONNECT_RECV_ID_QUIT:
        {
			qDebug() << "FTNoIR_Protocol_SC::processNextSimconnectEvent() says: Quit event!";
//            quit = 1;
            break;
        }

        default:
            break;
    }
}

//
// Return a name, if present the name from the Game, that is connected...
//
void FTNoIR_Protocol_SC::getNameFromGame( char *dest )
{   
	sprintf_s(dest, 99, "FSX");
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
	return new FTNoIR_Protocol_SC;
}

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
SCControls::SCControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	//connect(ui.cbxSelectPPJoyNumber, SIGNAL(currentIndexChanged(int)), this, SLOT(virtualJoystickSelected( int )));

	//for (int i = 1 ; i < 17; i++) {
	//	QString cbxText = QString("Virtual Joystick %1").arg(i);
	//	ui.cbxSelectPPJoyNumber->addItem(QIcon("images/PPJoy.ico"), cbxText);
	//}
	// Load the settings from the current .INI-file
	loadSettings();
}

//
// Destructor for server-dialog
//
SCControls::~SCControls() {
	qDebug() << "~SCControls() says: started";
}

void SCControls::Release()
{
    delete this;
}

//
// Initialize tracker-client-dialog
//
void SCControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

//
// OK clicked on server-dialog
//
void SCControls::doOK() {
	save();
	this->close();
}

// override show event
void SCControls::showEvent ( QShowEvent * event ) {
	loadSettings();
}

//
// Cancel clicked on server-dialog
//
void SCControls::doCancel() {
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
void SCControls::loadSettings() {

	settingsDirty = false;
}

//
// Save the current Settings to the currently 'active' INI-file.
//
void SCControls::save() {

	settingsDirty = false;
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
	return new SCControls;
}
