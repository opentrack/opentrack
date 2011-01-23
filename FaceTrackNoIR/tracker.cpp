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
*********************************************************************************/
/*
	Modifications (last one on top):
		20110109 - WVR: Added setZero option to define behaviour after STOP tracking via shortkey.
		20110104 - WVR: Removed a few nasty bugs (it was impossible to stop tracker without crash).
		20101224 - WVR: Removed the QThread inheritance of the Base Class for the protocol-servers.
						Again, this drastically simplifies the code in the protocols.
		20101217 - WVR: Created Base Class for the protocol-servers. This drastically simplifies
						the code needed here.
		20101024 - WVR: Added shortkey to disable/enable one or more axis during tracking.
		20101021 - WVR: Added FSUIPC server for FS2004.
		20101011 - WVR: Added SimConnect server.
		20101007 - WVR: Created 6DOF-curves and drastically changed the tracker for that.
						Also eliminated a 'glitch' in the process.
		20100607 - WVR: Re-installed Rotation Neutral Zone and improved reaction
						after 'start/stop'. MessageBeep when confidence is back...
		20100604 - WVR: Created structure for DOF-data and changed timing of
						ReceiveHeadPose end run().
		20100602 - WVR: Implemented EWMA-filtering, according to the example of
						Melchior Franz. Works like a charm...
		20100601 - WVR: Added DirectInput keyboard-handling. '=' used for center,
						'BACK' for start (+center)/stop.
		20100517 - WVR: Added upstream command(s) from FlightGear
		20100523 - WVR: Checkboxes to invert 6DOF's was implemented. Multiply by
						1 or (-1).
*/
#include "tracker.h"
#include "FaceTrackNoIR.h"

//
// Definitions for testing purposes
//
#define USE_HEADPOSE_CALLBACK
#define USE_DEBUG_CLIENT

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

// Flags
bool Tracker::confid = false;
bool Tracker::set_initial = false;
bool Tracker::do_tracking = true;
bool Tracker::do_center = false;
bool Tracker::do_inhibit = false;
bool Tracker::useFilter = false;
bool Tracker::setZero = true;
HANDLE Tracker::hTrackMutex = 0;

long Tracker::prevHeadPoseTime = 0;
THeadPoseDOF Tracker::Pitch;							// One structure for each of 6DOF's
THeadPoseDOF Tracker::Yaw;
THeadPoseDOF Tracker::Roll;
THeadPoseDOF Tracker::X;
THeadPoseDOF Tracker::Y;
THeadPoseDOF Tracker::Z;

TShortKey Tracker::CenterKey;							// ShortKey to Center headposition
TShortKey Tracker::StartStopKey;						// ShortKey to Start/stop tracking
TShortKey Tracker::InhibitKey;							// ShortKey to inhibit axis while tracking

/** constructor **/
Tracker::Tracker( int clientID, int facetrackerID ) {
importGetTracker getIT;
QLibrary *trackerLib;
importGetFilter getFilter;
QLibrary *filterLib;


	// Remember the selected client, from the ListBox
	// If the Tracker runs, this can NOT be changed...
	selectedClient = (FTNoIR_Client) clientID;
	selectedTracker = (FTNoIR_Face_Tracker) facetrackerID;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	Tracker::hTrackMutex = CreateMutexA(NULL, false, "HeadPose_mutex");

	switch (selectedTracker) {
		case FT_SM_FACEAPI:
			try {
				// Initialize the faceAPI Qt library
				sm::faceapi::qt::initialize();
				smLoggingSetFileOutputEnable( false );

				// Initialize the API
				faceapi_scope = new APIScope;

				// Create head-tracking engine v2 using first detected webcam
				CameraInfo::registerType(SM_API_CAMERA_TYPE_WDM);
				_engine = QSharedPointer<HeadTrackerV2>(new HeadTrackerV2());	

				// starts the faceapi engine
				_engine->start();
			} 
			catch (sm::faceapi::Error &e)
			{
				/* ERROR with camera */
				QMessageBox::warning(0,"faceAPI Error",e.what(),QMessageBox::Ok,QMessageBox::NoButton);
			}
			break;

		case FT_FTNOIR:
			trackerLib = new QLibrary("FTNoIR_Tracker_UDP.dll");
			
			getIT = (importGetTracker) trackerLib->resolve("GetTracker");
			
			if (getIT) {
				ITrackerPtr ptrXyz(getIT());
				if (ptrXyz)
				{
					pTracker = ptrXyz;
					qDebug() << "Tracker::setup Function Resolved!";
				}
			}
			else {
				QMessageBox::warning(0,"FaceTrackNoIR Error", "DLL not loaded",QMessageBox::Ok,QMessageBox::NoButton);
			}
			break;

		default:
			break;
	}


	//
	// Initialize all server-handles. Only start the server, that was selected in the GUI.
	//
	switch (selectedClient) {
		case FREE_TRACK:
			server_Game = QSharedPointer<FTServer>(new FTServer ( ));				// Create Free-track protocol-server
			break;

		case FLIGHTGEAR:
			server_Game = QSharedPointer<FGServer>(new FGServer ( this ));			// Create FlightGear protocol-server
			break;

		case FTNOIR:
			break;

		case PPJOY:
			server_Game = QSharedPointer<PPJoyServer>(new PPJoyServer ( this ));	// Create PPJoy protocol-server
			break;

		case TRACKIR:
			server_Game = QSharedPointer<FTIRServer>(new FTIRServer ( ));			// Create Fake-TIR protocol-server
			break;

		case SIMCONNECT:
			server_Game = QSharedPointer<SCServer>(new SCServer ( ));				// Create SimConnect protocol-server
			break;

		case FSUIPC:
			server_Game = QSharedPointer<FSUIPCServer>(new FSUIPCServer ( ));		// Create FSUIPC protocol-server
			break;

		default:
			// should never be reached
		break;
	}

#	ifdef USE_DEBUG_CLIENT
	debug_Client = QSharedPointer<ExcelServer>(new ExcelServer ( this ));		// Create Excel protocol-server
#   endif

	//
	// Load the DLL with the filter-logic and retrieve a pointer to the Filter-class.
	//
	filterLib = new QLibrary("FTNoIR_Filter_EWMA2.dll");
	
	getFilter = (importGetFilter) filterLib->resolve("GetFilter");
	if (getFilter) {
		IFilterPtr ptrXyz(getFilter());
		if (ptrXyz)
		{
			pFilter = ptrXyz;
			qDebug() << "Filter::setup Function Resolved!";
		}
	}
	else {
		QMessageBox::warning(0,"FaceTrackNoIR Error", "Filter-DLL not loaded",QMessageBox::Ok,QMessageBox::NoButton);
		return;
	}

	// Load the settings from the INI-file
	loadSettings();
}

/** destructor empty **/
Tracker::~Tracker() {

	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	if (isRunning()) {
		::WaitForSingleObject(m_WaitThread, INFINITE);
	}

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

	if (Tracker::hTrackMutex != 0) {
		::CloseHandle( Tracker::hTrackMutex );
	}

	if (selectedTracker == FT_SM_FACEAPI) {
		_engine->stop();
		smAPIQuit();
	}

	// Stop the started server(s)
	if (server_Game) {
		server_Game->deleteLater();
	}

#       ifdef USE_DEBUG_CLIENT
	debug_Client->deleteLater();		// Delete Excel protocol-server
#       endif
	
	qDebug() << "Tracker::~Tracker Finished...";

}

/** setting up the tracker engine **/
void Tracker::setup(QWidget *head, FaceTrackNoIR *parent) {
	bool DLL_Ok;

	// retrieve pointers to the User Interface and the main Application
	headPoseWidget = head;
	mainApp = parent;

	if (selectedTracker == FT_SM_FACEAPI) {
		//registers the faceapi callback for receiving headpose data **/
#       ifdef USE_HEADPOSE_CALLBACK
		registerHeadPoseCallback();
#       endif

		// some parameteres [optional]
		smHTSetHeadPosePredictionEnabled( _engine->handle(), false);
		smHTSetLipTrackingEnabled( _engine->handle(), false);
		smLoggingSetFileOutputEnable( false );
	}

	if (selectedTracker == FT_FTNOIR) {
		pTracker->StartTracker();
	}

	// set up the line edits for calling
	headXLine = headPoseWidget->findChild<QLineEdit *>("headXLine");
	headYLine = headPoseWidget->findChild<QLineEdit *>("headYLine");
	headZLine = headPoseWidget->findChild<QLineEdit *>("headZLine");

	headRotXLine = headPoseWidget->findChild<QLineEdit *>("headRotXLine");
	headRotYLine = headPoseWidget->findChild<QLineEdit *>("headRotYLine");
	headRotZLine = headPoseWidget->findChild<QLineEdit *>("headRotZLine");

	//
	// Check if the Protocol-server files were installed OK.
	// Some servers also create a memory-mapping, for Inter Process Communication.
	// The handle of the MainWindow is sent to 'The Game', so it can send a message back.
	//
	if (server_Game) {

		DLL_Ok = server_Game->checkServerInstallationOK( mainApp->winId() );
		if (!DLL_Ok) {
			QMessageBox::information(mainApp, "FaceTrackNoIR error", "Protocol is not (correctly) installed!");
		}

	}

#       ifdef USE_DEBUG_CLIENT
	DLL_Ok = debug_Client->checkServerInstallationOK( mainApp->winId() );		// Check installation
	if (!DLL_Ok) {
		QMessageBox::information(mainApp, "FaceTrackNoIR error", "Excel Protocol is not (correctly) installed!");
	}
#       endif

}

/** QThread run method @override **/
void Tracker::run() {
	/** Direct Input variables **/
	LPDIRECTINPUT8 din;								// the pointer to our DirectInput interface
	LPDIRECTINPUTDEVICE8 dinkeyboard;				// the pointer to the keyboard device
	BYTE keystate[256];								// the storage for the key-information
	HRESULT retAcquire;
	bool lastCenterKey = false;						// Remember state, to detect rising edge
	bool lastStartStopKey = false;
	bool lastInhibitKey = false;

	float rawrotX, rawrotY, rawrotZ;				// Locals...
	float rawposX, rawposY, rawposZ;
	//float new_camera_position.pitch, new_camera_position.yaw, new_camera_position.roll;							// Locals...
	//float new_camera_position.x, new_camera_position.y, new_camera_position.z;

	SYSTEMTIME now;
	long newHeadPoseTime;
	float dT;

	THeadPoseData current_camera_position;			// Used for filtering
	THeadPoseData target_camera_position;
	THeadPoseData new_camera_position;

#       ifndef USE_HEADPOSE_CALLBACK
	smEngineHeadPoseData head_pose;					// headpose from faceAPI
	smEngineHeadPoseData temp_head_pose;			// headpose from faceAPI
#       endif

	new_camera_position.x = 0.0f;
	new_camera_position.y = 0.0f;
	new_camera_position.z = 0.0f;
	new_camera_position.yaw = 0.0f;
	new_camera_position.pitch = 0.0f;
	new_camera_position.roll = 0.0f;

	//
	// Test some Filter-stuff
	//
	if (pFilter) {
		QString filterName;
		pFilter->getFilterFullName(&filterName);
		qDebug() << "Tracker::run() FilterName = " << filterName;
	}

	//
	// Setup the DirectInput for keyboard strokes
	//
    // create the DirectInput interface
    if (DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, 
						   (void**)&din, NULL) != DI_OK) {    // COM stuff, so we'll set it to NULL
		   qDebug() << "Tracker::setup DirectInput8 Creation failed!" << GetLastError();
	}

    // create the keyboard device
	if (din->CreateDevice(GUID_SysKeyboard, &dinkeyboard, NULL) != DI_OK) {
		   qDebug() << "Tracker::setup CreateDevice function failed!" << GetLastError();
	}

    // set the data format to keyboard format
	if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
		   qDebug() << "Tracker::setup SetDataFormat function failed!" << GetLastError();
	}

    // set the control you will have over the keyboard
	if (dinkeyboard->SetCooperativeLevel(mainApp->winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
		   qDebug() << "Tracker::setup SetCooperativeLevel function failed!" << GetLastError();
	}

	forever
	{

	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			dinkeyboard->Unacquire();			// Unacquire keyboard
			din->Release();						// Release DirectInput

			// Set event
			::SetEvent(m_WaitThread);
		   qDebug() << "Tracker::run terminated run()";
			return;
		}

		//
		// Check the keyboard
		//
		// get access if we don't have it already
		retAcquire = dinkeyboard->Acquire();
		if ( (retAcquire != DI_OK) && (retAcquire != S_FALSE) ) {
		   qDebug() << "Tracker::run Acquire function failed!" << GetLastError();
		}
		else {
			// get the input data
		   if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
			   qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
		   }
		   else {
				//
				// Check the state of the Start/Stop key
				//
				if ( isShortKeyPressed( &StartStopKey, &keystate[0] ) && (!lastStartStopKey) ) {
					Tracker::do_tracking = !Tracker::do_tracking;

					//
					// To start tracking again and to be at '0', execute Center command too
					//
					if (Tracker::do_tracking) {
						Tracker::do_center = true;

						Tracker::set_initial = false;
						Tracker::confid = false;

						Pitch.rawList.clear();
						Pitch.prevPos = 0.0f;
						Yaw.rawList.clear();
						Yaw.prevPos = 0.0f;
						Roll.rawList.clear();
						Roll.prevPos = 0.0f;
						X.rawList.clear();
						X.prevPos = 0.0f;
						Y.rawList.clear();
						Y.prevPos = 0.0f;
						Z.rawList.clear();
						Z.prevPos = 0.0f;

						_engine->start();
					}
					else {
						_engine->stop();
					}
					qDebug() << "Tracker::run() says StartStop pressed, do_tracking =" << Tracker::do_tracking;
				}
				lastStartStopKey = isShortKeyPressed( &StartStopKey, &keystate[0] );		// Remember

				//
				// Check the state of the Center key
				//
				if ( isShortKeyPressed( &CenterKey, &keystate[0] ) && (!lastCenterKey) ) {
					Tracker::do_center = true;
					qDebug() << "Tracker::run() says Center pressed";
				}
				lastCenterKey = isShortKeyPressed( &CenterKey, &keystate[0] );				// Remember

				//
				// Check the state of the Inhibit key
				//
				if ( isShortKeyPressed( &InhibitKey, &keystate[0] ) && (!lastInhibitKey) ) {
					Tracker::do_inhibit = !Tracker::do_inhibit;
					qDebug() << "Tracker::run() says Inhibit pressed";
					//
					// Execute Center command too, when inhibition ends.
					//
					if (!Tracker::do_inhibit) {
						Tracker::do_center = true;
					}
				}
				lastInhibitKey = isShortKeyPressed( &InhibitKey, &keystate[0] );		// Remember

		   }
		}

		if (WaitForSingleObject(Tracker::hTrackMutex, 100) == WAIT_OBJECT_0) {

#       ifndef USE_HEADPOSE_CALLBACK
            smReturnCode smret = smHTCurrentHeadPose(_engine->handle(), &temp_head_pose);
			memcpy(&head_pose, &temp_head_pose, sizeof(smEngineHeadPoseData));

			if ( head_pose.confidence > 0 ) {

				Tracker::confid = true;

				// Write the Raw headpose-data and add it to the RawList, for processing...
				addHeadPose( head_pose );
			} else {
				Tracker::confid = false;
			}
#       endif

			if (selectedTracker == FT_FTNOIR) {
				THeadPoseData newpose;
				pTracker->GiveHeadPoseData(&newpose);
				addHeadPose(newpose);
				Tracker::confid = true;
			}

			//
			// Get the System-time and substract the time from the previous call.
			// dT will be used for the EWMA-filter.
			//
			GetSystemTime ( &now );
			newHeadPoseTime = (((now.wHour * 3600) + (now.wMinute * 60) + now.wSecond) * 1000) + now.wMilliseconds;
			dT = (newHeadPoseTime - Tracker::prevHeadPoseTime) / 1000.0f;

			// Remember time for next call
			Tracker::prevHeadPoseTime = newHeadPoseTime;

			//if the confidence is good enough the headpose will be updated **/
			if (Tracker::confid) {

				//
				// Most games need an offset to the initial position and NOT the
				// absolute distance to the camera: so remember the initial distance
				// to substract that later...
				//
				if(Tracker::set_initial == false) {
					Tracker::Pitch.initial_headPos = Tracker::Pitch.headPos;
					Tracker::Yaw.initial_headPos = Tracker::Yaw.headPos;
					Tracker::Roll.initial_headPos = Tracker::Roll.headPos;
					Tracker::X.initial_headPos = Tracker::X.headPos;
					Tracker::Y.initial_headPos = Tracker::Y.headPos;
					Tracker::Z.initial_headPos = Tracker::Z.headPos;
					MessageBeep (MB_ICONASTERISK);
					Tracker::set_initial = true;
				}

				rawrotX = Tracker::Pitch.headPos- Tracker::Pitch.initial_headPos;	// degrees
				rawrotY = Tracker::Yaw.headPos- Tracker::Yaw.initial_headPos;
				rawrotZ = Tracker::Roll.headPos - Tracker::Roll.initial_headPos;
				rawposX = Tracker::X.headPos - Tracker::X.initial_headPos;										// centimeters
				rawposY = Tracker::Y.headPos - Tracker::Y.initial_headPos;
				rawposZ = Tracker::Z.headPos - Tracker::Z.initial_headPos;

				headRotXLine->setText(QString("%1").arg( rawrotX, 0, 'f', 1));		// show degrees
				headRotYLine->setText(QString("%1").arg( rawrotY, 0, 'f', 1));
				headRotZLine->setText(QString("%1").arg( rawrotZ, 0, 'f', 1));

				headXLine->setText(QString("%1").arg( rawposX, 0, 'f', 1));			// show centimeters
				headYLine->setText(QString("%1").arg( rawposY, 0, 'f', 1));
				headZLine->setText(QString("%1").arg( rawposZ, 0, 'f', 1));

			}

			//
			// If Center is pressed, copy the current values to the offsets.
			//
			if (Tracker::do_center && Tracker::set_initial) {
				Pitch.offset_headPos = getSmoothFromList( &Pitch.rawList )- Tracker::Pitch.initial_headPos;
				Yaw.offset_headPos = getSmoothFromList( &Yaw.rawList ) - Tracker::Yaw.initial_headPos;
				Roll.offset_headPos = getSmoothFromList( &Roll.rawList ) - Tracker::Roll.initial_headPos;
				X.offset_headPos = getSmoothFromList( &X.rawList ) - Tracker::X.initial_headPos;
				Y.offset_headPos = getSmoothFromList( &Y.rawList ) - Tracker::Y.initial_headPos;
				Z.offset_headPos = getSmoothFromList( &Z.rawList ) - Tracker::Z.initial_headPos;

				Tracker::do_center = false;
			}

			if (Tracker::do_tracking && Tracker::confid) {

				// Pitch
				current_camera_position = new_camera_position;			// Remember the latest position
				target_camera_position.x = X.headPos - X.offset_headPos - X.initial_headPos;
				target_camera_position.y = Y.headPos - Y.offset_headPos - Y.initial_headPos;
				target_camera_position.z = Z.headPos - Z.offset_headPos - Z.initial_headPos;
				target_camera_position.pitch = Pitch.headPos - Pitch.offset_headPos - Pitch.initial_headPos;
				target_camera_position.yaw   = Yaw.headPos - Yaw.offset_headPos - Yaw.initial_headPos;
				target_camera_position.roll  = Roll.headPos - Roll.offset_headPos - Roll.initial_headPos;

				if (Tracker::useFilter && pFilter) {
					pFilter->FilterHeadPoseData(&current_camera_position, &target_camera_position, &new_camera_position);
				}
				else {
					new_camera_position.x = getSmoothFromList( &X.rawList ) - X.offset_headPos - X.initial_headPos;
					new_camera_position.y = getSmoothFromList( &Y.rawList ) - Y.offset_headPos - Y.initial_headPos;
					new_camera_position.z = getSmoothFromList( &Z.rawList ) - Z.offset_headPos - Z.initial_headPos;
					new_camera_position.pitch = getSmoothFromList( &Pitch.rawList ) - Pitch.offset_headPos - Pitch.initial_headPos;
					new_camera_position.yaw = getSmoothFromList( &Yaw.rawList ) - Yaw.offset_headPos - Yaw.initial_headPos;
					new_camera_position.roll = getSmoothFromList( &Roll.rawList ) - Roll.offset_headPos - Roll.initial_headPos;
				}
				new_camera_position.x = X.invert * getOutputFromCurve(&X.curve, new_camera_position.x, X.NeutralZone, X.MaxInput);
				new_camera_position.y = Y.invert * getOutputFromCurve(&Y.curve, new_camera_position.y, Y.NeutralZone, Y.MaxInput);
				new_camera_position.z = Z.invert * getOutputFromCurve(&Z.curve, new_camera_position.z, Z.NeutralZone, Z.MaxInput);
				new_camera_position.pitch = Pitch.invert * getOutputFromCurve(&Pitch.curve, new_camera_position.pitch, Pitch.NeutralZone, Pitch.MaxInput);
				new_camera_position.yaw = Yaw.invert * getOutputFromCurve(&Yaw.curve, new_camera_position.yaw, Yaw.NeutralZone, Yaw.MaxInput);
				new_camera_position.roll = Roll.invert * getOutputFromCurve(&Roll.curve, new_camera_position.roll, Roll.NeutralZone, Roll.MaxInput);

				//
				// Reset value for the selected axis, if inhibition is active
				//
				if (Tracker::do_inhibit) {
					if (InhibitKey.doPitch) new_camera_position.pitch = 0.0f;
					if (InhibitKey.doYaw) new_camera_position.yaw = 0.0f;
					if (InhibitKey.doRoll) new_camera_position.roll = 0.0f;
					if (InhibitKey.doX) new_camera_position.x = 0.0f;
					if (InhibitKey.doY) new_camera_position.y = 0.0f;
					if (InhibitKey.doZ) new_camera_position.z = 0.0f;
				}

				//
				// Send the Virtual Pose to selected Protocol-Server
				//
				// Free-track
				if (selectedClient == FREE_TRACK) {
					server_Game->setHeadRotX( new_camera_position.pitch );					// degrees
					server_Game->setHeadRotY( new_camera_position.yaw );
					server_Game->setHeadRotZ( new_camera_position.roll );

					server_Game->setHeadPosX( new_camera_position.x );					// centimeters
					server_Game->setHeadPosY( new_camera_position.y );
					server_Game->setHeadPosZ( new_camera_position.z );
				}

				// All Protocol server(s)
				if (server_Game) {
					server_Game->setVirtRotX ( new_camera_position.pitch );				// degrees
					//server_Game->setVirtRotY ( new_camera_position.yaw );
					//server_Game->setVirtRotZ ( new_camera_position.roll );
					//server_Game->setVirtPosX ( new_camera_position.x );				// centimeters
					//server_Game->setVirtPosY ( new_camera_position.y );
					//server_Game->setVirtPosZ ( new_camera_position.z );
				}

#       ifdef USE_DEBUG_CLIENT
				debug_Client->setHeadRotX( Tracker::Pitch.headPos );	// degrees
				debug_Client->setHeadRotY( Tracker::Yaw.headPos );
				debug_Client->setHeadRotZ( Tracker::Roll.headPos );

				debug_Client->setHeadPosX( Tracker::X.headPos );		// centimeters
				debug_Client->setHeadPosY( Tracker::Y.headPos );
				debug_Client->setHeadPosZ( Tracker::Z.headPos );

				debug_Client->setVirtRotX ( new_camera_position.pitch );				// degrees
				debug_Client->setVirtRotY ( new_camera_position.yaw );
				debug_Client->setVirtRotZ ( new_camera_position.roll );
				debug_Client->setVirtPosX ( new_camera_position.x );				// centimeters
				debug_Client->setVirtPosY ( new_camera_position.y );
				debug_Client->setVirtPosZ ( new_camera_position.z );
#       endif


			}
			else {
				//
				// Go to initial position
				//
				if (server_Game && setZero) {
					server_Game->setVirtRotX ( 0.0f );
					server_Game->setVirtRotY ( 0.0f );
					server_Game->setVirtRotZ ( 0.0f );
					server_Game->setVirtPosX ( 0.0f );
					server_Game->setVirtPosY ( 0.0f );
					server_Game->setVirtPosZ ( 0.0f );
				}
			}
		}

#       ifdef USE_DEBUG_CLIENT
		debug_Client->confidence = Tracker::Pitch.confidence;
		debug_Client->newSample = Tracker::Pitch.newSample;
		debug_Client->smoothvalue = getSmoothFromList( &Pitch.rawList );
		debug_Client->prev_value = Tracker::Pitch.prevPos;
		debug_Client->dT = dT;
		debug_Client->sendHeadposeToGame();									// Log to Excel
		Tracker::Pitch.newSample = false;
#       endif

		ReleaseMutex(Tracker::hTrackMutex);
		server_Game->sendHeadposeToGame();

		//for lower cpu load 
		usleep(10000);
		yieldCurrentThread(); 
	}
}

/** registers the faceapi headpose callback function **/
void Tracker::registerHeadPoseCallback() {
	Q_ASSERT(_engine_handle);
	smReturnCode error = smHTRegisterHeadPoseCallback( _engine->handle(), 0, receiveHeadPose);
	//showErrorBox(0, "Register HeadPose Callback", error);
}

/** Callback function for head-pose - only static methods could be called **/
void Tracker::receiveHeadPose(void *,smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame)
{
	//
	// Perform actions, when valid data is received from faceAPI.
	//
	if (( head_pose.confidence > 0 ) && (WaitForSingleObject(Tracker::hTrackMutex, 100) == WAIT_OBJECT_0) ) {

		Tracker::confid = true;

		// Write the Raw headpose-data and add it to the RawList, for processing...
		addHeadPose( head_pose );
	} else {
		Tracker::confid = false;
	}

	ReleaseMutex(Tracker::hTrackMutex);

	// for lower cpu load
	msleep(10);
	yieldCurrentThread(); 
}

/** Add the headpose-data to the Lists **/
void Tracker::addHeadPose( smEngineHeadPoseData head_pose )
{
		// Pitch
		Tracker::Pitch.headPos = head_pose.head_rot.x_rads * 57.295781f;			// degrees
		addRaw2List ( &Pitch.rawList, Pitch.maxItems, Tracker::Pitch.headPos );
		Tracker::Pitch.confidence = head_pose.confidence;							// Just this one ...
		Tracker::Pitch.newSample = true;

		// Yaw
		Tracker::Yaw.headPos = head_pose.head_rot.y_rads * 57.295781f;				// degrees
		addRaw2List ( &Yaw.rawList, Yaw.maxItems, Tracker::Yaw.headPos );

		// Roll
		Tracker::Roll.headPos = head_pose.head_rot.z_rads * 57.295781f;				// degrees
		addRaw2List ( &Roll.rawList, Roll.maxItems, Tracker::Roll.headPos );

		// X-position
		Tracker::X.headPos = head_pose.head_pos.x * 100.0f;							// centimeters
		addRaw2List ( &X.rawList, X.maxItems, Tracker::X.headPos );

		// Y-position
		Tracker::Y.headPos = head_pose.head_pos.y * 100.0f;							// centimeters
		addRaw2List ( &Y.rawList, Y.maxItems, Tracker::Y.headPos );

		// Z-position (distance to camera, absolute!)
		Tracker::Z.headPos = head_pose.head_pos.z * 100.0f;							// centimeters
		addRaw2List ( &Z.rawList, Z.maxItems, Tracker::Z.headPos );
}

/** Add the headpose-data to the Lists **/
void Tracker::addHeadPose( THeadPoseData head_pose )
{
		// Pitch
		Tracker::Pitch.headPos = head_pose.pitch;									// degrees
		addRaw2List ( &Pitch.rawList, Pitch.maxItems, Tracker::Pitch.headPos );
//		Tracker::Pitch.confidence = head_pose.confidence;							// Just this one ...
		Tracker::Pitch.newSample = true;

		// Yaw
		Tracker::Yaw.headPos = head_pose.yaw;										// degrees
		addRaw2List ( &Yaw.rawList, Yaw.maxItems, Tracker::Yaw.headPos );

		// Roll
		Tracker::Roll.headPos = head_pose.roll;										// degrees
		addRaw2List ( &Roll.rawList, Roll.maxItems, Tracker::Roll.headPos );

		// X-position
		Tracker::X.headPos = head_pose.x;											// centimeters
		addRaw2List ( &X.rawList, X.maxItems, Tracker::X.headPos );

		// Y-position
		Tracker::Y.headPos = head_pose.y;											// centimeters
		addRaw2List ( &Y.rawList, Y.maxItems, Tracker::Y.headPos );

		// Z-position (distance to camera, absolute!)
		Tracker::Z.headPos = head_pose.z;											// centimeters
		addRaw2List ( &Z.rawList, Z.maxItems, Tracker::Z.headPos );
}

//
// Get the ProgramName from the Game and return it.
//
QString Tracker::getGameProgramName() {
QString str;

	str = server_Game->GetProgramName();
	return str;	
}

//
// Handle the command, send upstream by the game.
// Valid values are:
//		1	= reset Headpose
//
bool Tracker::handleGameCommand ( int command ) {

	qDebug() << "handleGameCommand says: Command =" << command;

	switch ( command ) {
		case 1:										// reset headtracker
			Tracker::do_center = true;
			break;
		default:
			break;
	}
	return false;
}

//
// Add the new Raw value to the QList.
// Remove the last item(s), depending on the set maximum list-items.
//
void Tracker::addRaw2List ( QList<float> *rawList, float maxIndex, float raw ) {
	//
	// Remove old values from the end of the QList.
	// If the setting for MaxItems was lowered, the QList is shortened here...
	//
	while (rawList->size() >= maxIndex) {
		rawList->removeLast();
	}
	
	//
	// Insert the newest at the beginning.
	//
	rawList->prepend ( raw );
}

//
// Get the Smoothed value from the QList.
//
float Tracker::getSmoothFromList ( QList<float> *rawList ) {
float sum = 0;

	if (rawList->isEmpty()) return 0.0f;

	//
	// Add the Raw values and divide.
	//
	for ( int i = 0; i < rawList->size(); i++) {
		sum += rawList->at(i);
	}
	return sum / rawList->size();
}

//
// Correct the Raw value, with the Neutral Zone supplied
//
float Tracker::getCorrectedNewRaw ( float NewRaw, float rotNeutral ) {

	//
	// Return 0, if NewRaw is within the Neutral Zone
	//
	if ( fabs( NewRaw ) < rotNeutral ) {
		return 0.0f;
	}

	//
	// NewRaw is outside the zone.
	// Substract rotNeutral from the NewRaw
	//
	if ( NewRaw > 0.0f ) {
		return (NewRaw - rotNeutral);
	}
	else {
		return (NewRaw + rotNeutral);				// Makes sense?
	}

}

//
// Implementation of an Exponentially Weighted Moving Average, used to serve as a low-pass filter.
// The code was adopted from Melchior Franz, who created it for FlightGear (aircraft.nas).
//
// The function takes the new value, the delta-time (sec) and a weighing coefficient (>0 and <1)
// All previous values are taken into account, the weight of this is determined by 'coeff'.
//
float Tracker::lowPassFilter ( float newvalue, float *oldvalue, float dt, float coeff) {
float c = 0.0f;
float fil = 0.0f;

	c = dt / (coeff + dt);
	fil = (newvalue * c) + (*oldvalue * (1 - c));
	*oldvalue = fil;

	return fil;
}

//
// Implementation of a Rate Limiter, used to eliminate spikes in the raw data.
//
// The function takes the new value, the delta-time (sec) and the positive max. slew-rate (engineering units/sec)
//
float Tracker::rateLimiter ( float newvalue, float *oldvalue, float dt, float max_rate) {
float rate = 0.0f;
float clamped_value = 0.0f;

	rate = (newvalue - *oldvalue) / dt;
	clamped_value = newvalue;									// If all is well, the newvalue is returned

	//
	// One max-rate is used for ramp-up and ramp-down
	// If the rate exceeds max_rate, return the maximum value that the max_rate allows
	//
	if (fabs(rate) > max_rate) {
		//
		// For ramp-down, apply a factor -1 to the max_rate
		//
		if (rate < 0.0f) {
			clamped_value = (-1.0f * dt * max_rate) + *oldvalue;
		}
		else {
			clamped_value = (dt * max_rate) + *oldvalue;
		}
	}
	*oldvalue = clamped_value;

	return clamped_value;
}

//
// Get the output from the curve.
//
float Tracker::getOutputFromCurve ( QPainterPath *curve, float input, float neutralzone, float maxinput ) {
float sign;

	sign = 1.0f;
	if (input < 0.0f) {
		sign = -1.0f;
	}

	//
	// Always return 0 inside the NeutralZone
	// Always return max. when input larger than expected
	//
	if (fabs(input) > maxinput) return sign * curve->pointAtPercent(1.0).x();

	//
	// Return the value, derived from the Bezier-curve
	//
	return sign * curve->pointAtPercent((fabs(input))/maxinput).x();
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void Tracker::loadSettings() {
int NeutralZone;
int sensYaw, sensPitch, sensRoll;
int sensX, sensY, sensZ;
QPointF point1, point2, point3, point4;

	qDebug() << "Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	//
	// Read the Tracking settings, to fill the curves.
	//
	iniFile.beginGroup ( "Tracking" );
	NeutralZone = iniFile.value ( "NeutralZone", 5 ).toInt();
	sensYaw = iniFile.value ( "sensYaw", 100 ).toInt();
	sensPitch = iniFile.value ( "sensPitch", 100 ).toInt();
	sensRoll = iniFile.value ( "sensRoll", 100 ).toInt();
	sensX = iniFile.value ( "sensX", 100 ).toInt();
	sensY = iniFile.value ( "sensY", 100 ).toInt();
	sensZ = iniFile.value ( "sensZ", 100 ).toInt();
	iniFile.endGroup ();

	//
	// Read the curve-settings from the file. Use the (deprecated) settings, if the curves are not there.
	//
	iniFile.beginGroup ( "Curves" );

	//
	// Create a new path and assign it to the curve.
	//
	getCurvePoints( &iniFile, "Yaw_", &point1, &point2, &point3, &point4, NeutralZone, sensYaw, 50, 180 );
	QPainterPath newYawCurve;
	newYawCurve.moveTo( QPointF(0,0) );
	newYawCurve.lineTo( point1 );
	newYawCurve.cubicTo(point2, point3, point4);

	Yaw.NeutralZone = point1.y();							// Get the Neutral Zone
	Yaw.MaxInput = point4.y();								// Get Maximum Input
	Yaw.curve = newYawCurve;

	qDebug() << "loadSettings says: curve-elementcount = " << Yaw.curve.elementCount();

	// Pitch
	getCurvePoints( &iniFile, "Pitch_", &point1, &point2, &point3, &point4, NeutralZone, sensPitch, 50, 180 );
	QPainterPath newPitchCurve;
	newPitchCurve.moveTo( QPointF(0,0) );
	newPitchCurve.lineTo( point1 );
    newPitchCurve.cubicTo(point2, point3, point4);

	Pitch.NeutralZone = point1.y();							// Get the Neutral Zone
	Pitch.MaxInput = point4.y();							// Get Maximum Input
	Pitch.curve = newPitchCurve;

	// Roll
	getCurvePoints( &iniFile, "Roll_", &point1, &point2, &point3, &point4, NeutralZone, sensRoll, 50, 180 );
	QPainterPath newRollCurve;
	newRollCurve.moveTo( QPointF(0,0) );
	newRollCurve.lineTo( point1 );
    newRollCurve.cubicTo(point2, point3, point4);

	Roll.NeutralZone = point1.y();							// Get the Neutral Zone
	Roll.MaxInput = point4.y();								// Get Maximum Input
    Roll.curve = newRollCurve;

	// X
	getCurvePoints( &iniFile, "X_", &point1, &point2, &point3, &point4, NeutralZone, sensX, 50, 180 );
	QPainterPath newXCurve;
	newXCurve.moveTo( QPointF(0,0) );
	newXCurve.lineTo( point1 );
    newXCurve.cubicTo(point2, point3, point4);

	X.NeutralZone = point1.y();								// Get the Neutral Zone
	X.MaxInput = point4.y();								// Get Maximum Input
    X.curve = newXCurve;

	// Y
	getCurvePoints( &iniFile, "Y_", &point1, &point2, &point3, &point4, NeutralZone, sensY, 50, 180 );
	QPainterPath newYCurve;
	newYCurve.moveTo( QPointF(0,0) );
	newYCurve.lineTo( point1 );
    newYCurve.cubicTo(point2, point3, point4);

	Y.NeutralZone = point1.y();								// Get the Neutral Zone
	Y.MaxInput = point4.y();								// Get Maximum Input
    Y.curve = newYCurve;

	// Z
	getCurvePoints( &iniFile, "Z_", &point1, &point2, &point3, &point4, NeutralZone, sensZ, 50, 180 );
	QPainterPath newZCurve;
	newZCurve.moveTo( QPointF(0,0) );
	newZCurve.lineTo( point1 );
    newZCurve.cubicTo(point2, point3, point4);

	Z.NeutralZone = point1.y();								// Get the Neutral Zone
	Z.MaxInput = point4.y();								// Get Maximum Input
    Z.curve = newZCurve;

	iniFile.endGroup ();

	//
	// Read the keyboard shortcuts.
	//
	iniFile.beginGroup ( "KB_Shortcuts" );
	
	// Center key
	CenterKey.keycode = iniFile.value ( "Keycode_Center", 0 ).toInt();
	CenterKey.shift = iniFile.value ( "Shift_Center", 0 ).toBool();
	CenterKey.ctrl = iniFile.value ( "Ctrl_Center", 0 ).toBool();
	CenterKey.alt = iniFile.value ( "Alt_Center", 0 ).toBool();

	// StartStop key
	StartStopKey.keycode = iniFile.value ( "Keycode_StartStop", 0 ).toInt();
	StartStopKey.shift = iniFile.value ( "Shift_StartStop", 0 ).toBool();
	StartStopKey.ctrl = iniFile.value ( "Ctrl_StartStop", 0 ).toBool();
	StartStopKey.alt = iniFile.value ( "Alt_StartStop", 0 ).toBool();
	setZero = iniFile.value ( "SetZero", 1 ).toBool();

	// Inhibit key
	InhibitKey.keycode = iniFile.value ( "Keycode_Inhibit", 0 ).toInt();
	InhibitKey.shift = iniFile.value ( "Shift_Inhibit", 0 ).toBool();
	InhibitKey.ctrl = iniFile.value ( "Ctrl_Inhibit", 0 ).toBool();
	InhibitKey.alt = iniFile.value ( "Alt_Inhibit", 0 ).toBool();
	InhibitKey.doPitch = iniFile.value ( "Inhibit_Pitch", 0 ).toBool();
	InhibitKey.doYaw = iniFile.value ( "Inhibit_Yaw", 0 ).toBool();
	InhibitKey.doRoll = iniFile.value ( "Inhibit_Roll", 0 ).toBool();
	InhibitKey.doX = iniFile.value ( "Inhibit_X", 0 ).toBool();
	InhibitKey.doY = iniFile.value ( "Inhibit_Y", 0 ).toBool();
	InhibitKey.doZ = iniFile.value ( "Inhibit_Z", 0 ).toBool();

	iniFile.endGroup ();
}

//
// Determine if the ShortKey (incl. CTRL, SHIFT and/or ALT) is pressed.
//
bool Tracker::isShortKeyPressed( TShortKey *key, BYTE *keystate ){
bool shift;
bool ctrl;
bool alt;

	//
	// First, check if the right key is pressed. If so, check the modifiers
	//
	if (keystate[key->keycode] & 0x80) {
		shift = ( (keystate[DIK_LSHIFT] & 0x80) || (keystate[DIK_RSHIFT] & 0x80) );
		ctrl  = ( (keystate[DIK_LCONTROL] & 0x80) || (keystate[DIK_RCONTROL] & 0x80) );
		alt   = ( (keystate[DIK_LALT] & 0x80) || (keystate[DIK_RALT] & 0x80) );
		
		//
		// If one of the modifiers is needed and not pressed, return false.
		//
		if (key->shift && !shift) return false;
		if (key->ctrl && !ctrl) return false;
		if (key->alt && !alt) return false;

		//
		// All is well!
		//
		return true;
	}
	else {
		return false;
	}
}
