/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
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
		20130201 - WVR: Remove the Protocol, when stopping the Thread.
		20121215 - WVR: Fixed crash after message: protocol not installed correctly... by terminating the thread.
		20120921 - WVR: Fixed centering when no filter is selected.
		20120917 - WVR: Added Mouse-buttons to ShortKeys.
		20120827 - WVR: Signal tracking = false to Curve-widget(s) when quitting run(). Also when Alternative Pitch curve is used.
		20120805 - WVR: The FunctionConfig-widget is used to configure the Curves. It was tweaked some more, because the Accela filter now also
						uses the Curve(s). ToDo: make the ranges configurable by the user. Development on the Toradex IMU makes us realize, that
						a fixed input-range may not be so handy after all..
		20120427 - WVR: The Protocol-code was already in separate DLLs, but the ListBox was still filled ´statically´. Now, a Dir() of the
						EXE-folder is done, to locate Protocol-DLLs. The Icons were also moved to the DLLs
		20120317 - WVR: The Filter and Tracker-code was moved to separate DLLs. The calling-method
						was changed accordingly.
						The face-tracker member-functions NotifyZeroed and refreshVideo were added, as 
						requested by Stanislaw.
		20110411 - WVR: Finished moving all Protocols to separate C++ projects. Every protocol now
						has it's own Class, that's inside it's own DLL. This reduces the size of the program,
						makes it more structured and enables a more sophisticated installer.
		20110328 - WVR: Changed the camera-structs into class-instances. This makes initialisation
						easier and hopefully solves the remaining 'start-up problem'.
		20110313 - WVR: Removed 'set_initial'. Less is more.
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

// Flags
bool Tracker::confid = false;
bool Tracker::do_tracking = true;
bool Tracker::do_center = false;
bool Tracker::do_inhibit = false;
bool Tracker::do_game_zero = false;
bool Tracker::do_axis_reverse = false;

bool Tracker::setZero = true;
bool Tracker::setEngineStop = true;
HANDLE Tracker::hTrackMutex = 0;

bool Tracker::useAxisReverse = false;							// Use Axis Reverse
float Tracker::YawAngle4ReverseAxis = 40.0f;					// Axis Reverse settings
float Tracker::Z_Pos4ReverseAxis = -20.0f;
float Tracker::Z_PosWhenReverseAxis = 50.0f;


T6DOF Tracker::current_camera(0,0,0,0,0,0);						// Used for filtering
T6DOF Tracker::target_camera(0,0,0,0,0,0);
T6DOF Tracker::new_camera(0,0,0,0,0,0);
T6DOF Tracker::output_camera(0,0,0,0,0,0);						// Position sent to game protocol

THeadPoseDOF Tracker::Pitch("PitchUp", "PitchDown", 50, 180, 50, 90);	// One structure for each of 6DOF's
THeadPoseDOF Tracker::Yaw("Yaw", "", 50, 180);
THeadPoseDOF Tracker::Roll("Roll", "", 50, 180);
THeadPoseDOF Tracker::X("X","", 50, 180);
THeadPoseDOF Tracker::Y("Y","", 50, 180);
THeadPoseDOF Tracker::Z("Z","", 50, 180);

TShortKey Tracker::CenterKey;									// ShortKey to Center headposition
TShortKey Tracker::StartStopKey;								// ShortKey to Start/stop tracking
TShortKey Tracker::InhibitKey;									// ShortKey to inhibit axis while tracking
TShortKey Tracker::GameZeroKey;									// ShortKey to Set Game Zero
bool Tracker::DisableBeep = false;								// Disable beep when center
//TShortKey Tracker::AxisReverseKey;							// ShortKey to start/stop axis reverse while tracking

int Tracker::CenterMouseKey;									// ShortKey to Center headposition
int Tracker::StartStopMouseKey;									// ShortKey to Start/stop tracking
int Tracker::InhibitMouseKey;									// ShortKey to inhibit axis while tracking
int Tracker::GameZeroMouseKey;									// ShortKey to Set Game Zero

//ITrackerPtr Tracker::pTracker;								// Pointer to Tracker instance (in DLL)
IProtocolPtr Tracker::pProtocol;								// Pointer to Protocol instance (in DLL)
IFilterPtr Tracker::pFilter;									// Pointer to Filter instance (in DLL)


/** constructor **/
Tracker::Tracker( FaceTrackNoIR *parent ) {
QString libName;
importGetTracker getIT;
QLibrary *trackerLib;
importGetFilter getFilter;
QLibrary *filterLib;
importGetProtocol getProtocol;
QLibrary *protocolLib;
QFrame *video_frame;

	// Retieve the pointer to the parent
	mainApp = parent;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	Tracker::hTrackMutex = CreateMutexA(NULL, false, "HeadPose_mutex");

	//
	// Initialize the headpose-data
	//
	Tracker::Yaw.initHeadPoseData();
	Tracker::Pitch.initHeadPoseData();
	Tracker::Roll.initHeadPoseData();
	Tracker::X.initHeadPoseData();
	Tracker::Y.initHeadPoseData();
	Tracker::Z.initHeadPoseData();

	//
	// Locate the video-frame, for the DLL
	//
	video_frame = 0;
	video_frame = mainApp->getVideoWidget();
	qDebug() << "Tracker::Tracker VideoFrame = " << video_frame;

	//
	// Load the Tracker-engine DLL, get the tracker-class from it and do stuff...
	//
	pTracker = NULL;
	libName = mainApp->getCurrentTrackerName();
	if (!libName.isEmpty()) {
		trackerLib = new QLibrary(libName);
		getIT = (importGetTracker) trackerLib->resolve("GetTracker");
		qDebug() << "Tracker::Tracker libName = " << libName;
			
		if (getIT) {
			ITracker *ptrXyz(getIT());							// Get the Class
			if (ptrXyz)
			{
				pTracker = ptrXyz;
				pTracker->Initialize( video_frame );
				qDebug() << "Tracker::setup Function Resolved!";
			}
		}
		else {
			QMessageBox::warning(0,"FaceTrackNoIR Error", libName + " DLL not loaded",QMessageBox::Ok,QMessageBox::NoButton);
		}
	}
	//
	// Load the Tracker-engine DLL, get the tracker-class from it and do stuff...
	//
	pSecondTracker = NULL;
	libName = mainApp->getSecondTrackerName();
	if ((!libName.isEmpty()) && (libName != "None")) {
		trackerLib = new QLibrary(libName);
		getIT = (importGetTracker) trackerLib->resolve("GetTracker");
			
		if (getIT) {
			ITracker *ptrXyz(getIT());							// Get the Class
			if (ptrXyz)
			{
				pSecondTracker = ptrXyz;
				pSecondTracker->Initialize( NULL );
				qDebug() << "Tracker::setup Function Resolved!";
			}
		}
		else {
			QMessageBox::warning(0,"FaceTrackNoIR Error", libName + " DLL not loaded",QMessageBox::Ok,QMessageBox::NoButton);
		}
	}

	//
	// Load the DLL with the protocol-logic and retrieve a pointer to the Protocol-class.
	//
	libName = mainApp->getCurrentProtocolName();
	if (!libName.isEmpty()) {
		protocolLib = new QLibrary(libName);
		getProtocol = (importGetProtocol) protocolLib->resolve("GetProtocol");
		if (getProtocol) {
			IProtocolPtr ptrXyz(getProtocol());
			if (ptrXyz)
			{
				pProtocol = ptrXyz;
				pProtocol->Initialize();
				qDebug() << "Protocol::setup Function Resolved!";
			}
		}
		else {
			QMessageBox::warning(0,"FaceTrackNoIR Error", "Protocol-DLL not loaded",QMessageBox::Ok,QMessageBox::NoButton);
			return;
		}
	}

	//
	// Load the DLL with the filter-logic and retrieve a pointer to the Filter-class.
	//
	pFilter = NULL;
	libName = mainApp->getCurrentFilterName();

	if ((!libName.isEmpty()) && (libName != "None")) {
		filterLib = new QLibrary(libName);
		
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
	}

	// Load the settings from the INI-file
	loadSettings();
}

/** destructor empty **/
Tracker::~Tracker() {

	// Stop the Tracker(s)
	if (pTracker) {
		pTracker->StopTracker( true );
	}
	if (pSecondTracker) {
		pSecondTracker->StopTracker( true );
	}

	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	if (isRunning()) {
		::WaitForSingleObject(m_WaitThread, INFINITE);
	}

	//
	// Remove the Tracker
	// 20120615, WVR: As suggested by Stanislaw
	if (pTracker) {
		delete pTracker;
		pTracker = NULL;
	}
	if (pSecondTracker) {
		delete pSecondTracker;
		pSecondTracker = NULL;
	}

	//
	// Remove the Protocol
	//
	if (pProtocol) {
		delete pProtocol;
		pProtocol = NULL;
	}

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

	if (Tracker::hTrackMutex != 0) {
		::CloseHandle( Tracker::hTrackMutex );
	}

#       ifdef USE_DEBUG_CLIENT
	debug_Client->deleteLater();		// Delete Excel protocol-server
#       endif
	
	qDebug() << "Tracker::~Tracker Finished...";

}

/** setting up the tracker engine **/
void Tracker::setup() {
	bool DLL_Ok;

	// retrieve pointers to the User Interface and the main Application
	if (pTracker) {
		pTracker->StartTracker( mainApp->winId() );
	}
	if (pSecondTracker) {
		pSecondTracker->StartTracker( mainApp->winId() );
	}

	//
	// Check if the Protocol-server files were installed OK.
	// Some servers also create a memory-mapping, for Inter Process Communication.
	// The handle of the MainWindow is sent to 'The Game', so it can send a message back.
	//
	if (pProtocol) {

		DLL_Ok = pProtocol->checkServerInstallationOK( mainApp->winId() );
		if (!DLL_Ok) {
			// Trigger thread to stop
			::SetEvent(m_StopThread);
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
//
// The DirectX stuff was found here: http://www.directxtutorial.com/tutorial9/e-directinput/dx9e2.aspx
//
LPDIRECTINPUT8 din;								// the pointer to our DirectInput interface
LPDIRECTINPUTDEVICE8 dinkeyboard;				// the pointer to the keyboard device
LPDIRECTINPUTDEVICE8 dinmouse;					// the pointer to the mouse device
BYTE keystate[256];								// the storage for the key-information
DIMOUSESTATE mousestate;						// the storage for the mouse-information
HRESULT retAcquire;
bool lastCenterKey = false;						// Remember state, to detect rising edge
bool lastStartStopKey = false;
bool lastInhibitKey = false;
bool lastGameZeroKey = false;

bool lastCenterMouseKey = false;				// Remember state, to detect rising edge
bool lastStartStopMouseKey = false;
bool lastInhibitMouseKey = false;
bool lastGameZeroMouseKey = false;

bool waitAxisReverse = false;
bool waitThroughZero = false;
double actualYaw = 0.0f;
double actualZ = 0.0f;
T6DOF offset_camera(0,0,0,0,0,0);
T6DOF gamezero_camera(0,0,0,0,0,0);
T6DOF gameoutput_camera(0,0,0,0,0,0);

bool bInitialCenter1 = true;
bool bInitialCenter2 = true;
bool bTracker1Confid = false;
bool bTracker2Confid = false;

	Tracker::do_tracking = true;				// Start initially
	Tracker::do_center = false;					// Center initially

	//
	// Test some Filter-stuff
	//
	if (pFilter) {
		QString filterName;
		//pFilter->getFullName(&filterName);
		//qDebug() << "Tracker::run() FilterName = " << filterName;
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
    // create the mouse device
    din->CreateDevice(GUID_SysMouse, &dinmouse, NULL);

    // set the data format to keyboard format
	if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
		   qDebug() << "Tracker::setup SetDataFormat function failed!" << GetLastError();
	}
    // set the data format to mouse format
    dinmouse->SetDataFormat(&c_dfDIMouse);

    // set the control you will have over the keyboard
	if (dinkeyboard->SetCooperativeLevel(mainApp->winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
		   qDebug() << "Tracker::setup SetCooperativeLevel function failed!" << GetLastError();
	}
    // set the control you will have over the mouse
    dinmouse->SetCooperativeLevel(mainApp->winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

	forever
	{

	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			dinkeyboard->Unacquire();			// Unacquire keyboard
			dinkeyboard->Release();
			din->Release();						// Release DirectInput

			// Set event
			::SetEvent(m_WaitThread);
			qDebug() << "Tracker::run terminated run()";
			X.curvePtr->setTrackingActive( false );
			Y.curvePtr->setTrackingActive( false );
			Z.curvePtr->setTrackingActive( false );
			Yaw.curvePtr->setTrackingActive( false );
			Pitch.curvePtr->setTrackingActive( false );
			Pitch.curvePtrAlt->setTrackingActive( false );
			Roll.curvePtr->setTrackingActive( false );

			return;
		}
    
		//
		// Check the mouse
		//
		// get access if we don't have it already
		retAcquire = dinmouse->Acquire();
		if ( (retAcquire != DI_OK) && (retAcquire != S_FALSE) ) {
		   qDebug() << "Tracker::run Acquire function failed!" << GetLastError();
		}
		else {
			if (dinmouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mousestate) != DI_OK) {
			   qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
			}
			else {
				//
				// Check the state of the StartStop MouseKey
				//
				if ( isMouseKeyPressed( &StartStopMouseKey, &mousestate ) && (!lastStartStopMouseKey) ) {
					Tracker::do_tracking = !Tracker::do_tracking;

					//
					// To start tracking again and to be at '0', execute Center command too
					//
					if (Tracker::do_tracking) {
						Tracker::confid = false;
						if (pTracker) {
							pTracker->StartTracker( mainApp->winId() );
						}
						if (pSecondTracker) {
							pSecondTracker->StartTracker( mainApp->winId() );
						}
					}
					else {
						if (setEngineStop) {						// Only stop engine when option is checked
							if (pTracker) {
								pTracker->StopTracker( false );
							}
							if (pSecondTracker) {
								pSecondTracker->StopTracker( false );
							}
						}
					}
					qDebug() << "Tracker::run() says StartStop pressed, do_tracking =" << Tracker::do_tracking;
				}
				lastStartStopMouseKey = isMouseKeyPressed( &StartStopMouseKey, &mousestate );				// Remember

				//
				// Check the state of the Center MouseKey
				//
				if ( isMouseKeyPressed( &CenterMouseKey, &mousestate ) && (!lastCenterMouseKey) ) {
					Tracker::do_center = true;
					qDebug() << "Tracker::run() says Center MouseKey pressed";
				}
				lastCenterMouseKey = isMouseKeyPressed( &CenterMouseKey, &mousestate );						// Remember

				//
				// Check the state of the GameZero MouseKey
				//
				if ( isMouseKeyPressed( &GameZeroMouseKey, &mousestate ) && (!lastGameZeroMouseKey) ) {
					Tracker::do_game_zero = true;
					qDebug() << "Tracker::run() says GameZero MouseKey pressed";
				}
				lastGameZeroMouseKey = isMouseKeyPressed( &GameZeroMouseKey, &mousestate );					// Remember

				//
				// Check the state of the Inhibit MouseKey
				//
				if ( isMouseKeyPressed( &InhibitMouseKey, &mousestate ) && (!lastInhibitMouseKey) ) {
					Tracker::do_inhibit = !Tracker::do_inhibit;
					qDebug() << "Tracker::run() says Inhibit MouseKey pressed";
					//
					// Execute Center command too, when inhibition ends.
					//
					if (!Tracker::do_inhibit) {
						Tracker::do_center = true;
					}
				}
				lastInhibitMouseKey = isMouseKeyPressed( &InhibitMouseKey, &mousestate );					// Remember
			}
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
						Tracker::confid = false;
						if (pTracker) {
							pTracker->StartTracker( mainApp->winId() );
						}
						if (pSecondTracker) {
							pSecondTracker->StartTracker( mainApp->winId() );
						}
					}
					else {
						if (setEngineStop) {						// Only stop engine when option is checked
							if (pTracker) {
								pTracker->StopTracker( false );
							}
							if (pSecondTracker) {
								pSecondTracker->StopTracker( false );
							}
						}
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
				// Check the state of the GameZero key
				//
				if ( isShortKeyPressed( &GameZeroKey, &keystate[0] ) && (!lastGameZeroKey) ) {
					Tracker::do_game_zero = true;
					qDebug() << "Tracker::run() says GameZero pressed";
				}
				lastGameZeroKey = isShortKeyPressed( &GameZeroKey, &keystate[0] );			// Remember

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

		//
		// Reset the 'wait' flag. Moving above 90 with the key pressed, will (de-)activate Axis Reverse.
		//
//		qDebug() << "Tracker::run() says actualZ = " << actualZ << ", terwijl Z_Pos4 = " << Z_Pos4ReverseAxis;
		if (useAxisReverse) {
			Tracker::do_axis_reverse = ((fabs(actualYaw) > YawAngle4ReverseAxis) && (actualZ < Z_Pos4ReverseAxis));
		}
		else {
			Tracker::do_axis_reverse = false;
		}


		if (WaitForSingleObject(Tracker::hTrackMutex, 100) == WAIT_OBJECT_0) {

			THeadPoseData newpose;
			newpose.pitch = 0.0f;
			newpose.roll = 0.0f;
			newpose.yaw = 0.0f;
			newpose.x = 0.0f;
			newpose.y = 0.0f;
			newpose.z = 0.0f;

			//
			// The second tracker serves as 'secondary'. So if an axis is written by the second tracker it CAN be overwritten by the Primary tracker.
			// This is enforced by the sequence below.
			//
			if (pSecondTracker) {
				bTracker2Confid = pSecondTracker->GiveHeadPoseData(&newpose);
			}
			else {
				bTracker2Confid = false;
				bInitialCenter2 = false;
			}
			if (pTracker) {
				bTracker1Confid = pTracker->GiveHeadPoseData(&newpose);
			}
			else {
				bTracker1Confid = false;
				bInitialCenter1 = false;
			}

			Tracker::confid = (bTracker1Confid || bTracker2Confid);
			if ( Tracker::confid ) {
				addHeadPose(newpose);
			}

			//
			// If Center is pressed, copy the current values to the offsets.
			//
			if ((Tracker::do_center) || ((bInitialCenter1 && bTracker1Confid ) || (bInitialCenter2 && bTracker2Confid)))  {
				
				if (!DisableBeep) {
					MessageBeep (MB_ICONASTERISK);				// Acknowledge the key-press with a beep.
				}
				if (pTracker && bTracker1Confid) {
					pTracker->notifyCenter();					// Send 'center' to the tracker
					bInitialCenter1 = false;
				}
				if (pSecondTracker && bTracker2Confid) {
					pSecondTracker->notifyCenter();				// Send 'center' to the second tracker
					bInitialCenter2 = false;
				}

				//
				// Only copy valid values
				//
				if (Tracker::confid) {

					offset_camera.x     = getSmoothFromList( &X.rawList );
					offset_camera.y     = getSmoothFromList( &Y.rawList );
					offset_camera.z     = getSmoothFromList( &Z.rawList );
					offset_camera.pitch = getSmoothFromList( &Pitch.rawList );
					offset_camera.yaw   = getSmoothFromList( &Yaw.rawList );
					offset_camera.roll  = getSmoothFromList( &Roll.rawList );
				}

				Tracker::do_center = false;
			}

			//
			// If Set Game Zero is pressed, copy the current values to the offsets.
			// Change requested by Stanislaw
			//
			if (Tracker::confid && Tracker::do_game_zero) {
				if (pTracker) {
					if (!pTracker->notifyZeroed())
						gamezero_camera = gameoutput_camera;
				}
//				gamezero_camera = gameoutput_camera;

				Tracker::do_game_zero = false;
			}

			if (Tracker::do_tracking && Tracker::confid) {

				// get values
				target_camera.x     = getSmoothFromList( &X.rawList );
				target_camera.y     = getSmoothFromList( &Y.rawList );
				target_camera.z     = getSmoothFromList( &Z.rawList );
				target_camera.pitch = getSmoothFromList( &Pitch.rawList );
				target_camera.yaw   = getSmoothFromList( &Yaw.rawList );
				target_camera.roll  = getSmoothFromList( &Roll.rawList );

				// do the centering
				target_camera = target_camera - offset_camera;

				//
				// Use advanced filtering, when a filter was selected.
				//
				if (pFilter) {
					pFilter->FilterHeadPoseData(&current_camera, &target_camera, &new_camera, Tracker::Pitch.newSample);
				}
				else {
					new_camera = target_camera;
				}
				output_camera.x = X.invert * X.curvePtr->getValue(new_camera.x);
				output_camera.y = Y.invert * Y.curvePtr->getValue(new_camera.y);
				output_camera.z = Z.invert * Z.curvePtr->getValue(new_camera.z);

				//
				// Determine, which curve (Up or Down) must be used for Pitch
				//
				bool altp = (new_camera.pitch < 0);
				if (altp) {
					output_camera.pitch = Pitch.invert * Pitch.curvePtrAlt->getValue(new_camera.pitch);
					Pitch.curvePtr->setTrackingActive( false );
					Pitch.curvePtrAlt->setTrackingActive( true );
				}
				else {
					output_camera.pitch = Pitch.invert * Pitch.curvePtr->getValue(new_camera.pitch);
					Pitch.curvePtr->setTrackingActive( true );
					Pitch.curvePtrAlt->setTrackingActive( false );
				}
				output_camera.yaw = Yaw.invert * Yaw.curvePtr->getValue(new_camera.yaw);
				output_camera.roll = Roll.invert * Roll.curvePtr->getValue(new_camera.roll);

				X.curvePtr->setTrackingActive( true );
				Y.curvePtr->setTrackingActive( true );
				Z.curvePtr->setTrackingActive( true );
				Yaw.curvePtr->setTrackingActive( true );
				Roll.curvePtr->setTrackingActive( true );

				//
				// Reverse Axis.
				//
				actualYaw = output_camera.yaw;					// Save the actual Yaw, otherwise we can't check for +90
				actualZ = output_camera.z;						// Also the Z
				if (Tracker::do_axis_reverse) {
					output_camera.z = Z_PosWhenReverseAxis;	// Set the desired Z-position
				}

				//
				// Reset value for the selected axis, if inhibition is active
				//
				if (Tracker::do_inhibit) {
					if (InhibitKey.doPitch) output_camera.pitch = 0.0f;
					if (InhibitKey.doYaw) output_camera.yaw = 0.0f;
					if (InhibitKey.doRoll) output_camera.roll = 0.0f;
					if (InhibitKey.doX) output_camera.x = 0.0f;
					if (InhibitKey.doY) output_camera.y = 0.0f;
					if (InhibitKey.doZ) output_camera.z = 0.0f;
				}

				//
				// Send the headpose to the game
				//
				if (pProtocol) {
					gameoutput_camera = output_camera + gamezero_camera;
					pProtocol->sendHeadposeToGame( &gameoutput_camera, &newpose );	// degrees & centimeters
				}
			}
			else {
				//
				// Go to initial position
				//
				if (pProtocol && setZero) {
					output_camera.pitch = 0.0f;
					output_camera.yaw = 0.0f;
					output_camera.roll = 0.0f;
					output_camera.x = 0.0f;
					output_camera.y = 0.0f;
					output_camera.z = 0.0f;
					gameoutput_camera = output_camera + gamezero_camera;
					pProtocol->sendHeadposeToGame( &gameoutput_camera, &newpose );				// degrees & centimeters
				}
				X.curvePtr->setTrackingActive( false );
				Y.curvePtr->setTrackingActive( false );
				Z.curvePtr->setTrackingActive( false );
				Yaw.curvePtr->setTrackingActive( false );
				Pitch.curvePtr->setTrackingActive( false );
				Pitch.curvePtrAlt->setTrackingActive( false );
				Roll.curvePtr->setTrackingActive( false );
			}
		}

		Tracker::Pitch.newSample = false;
		ReleaseMutex(Tracker::hTrackMutex);

		//for lower cpu load 
		usleep(10000);
		yieldCurrentThread(); 
	}
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
char dest[100];

	str = QString("No protocol active?");
	if (pProtocol) {
		pProtocol->getNameFromGame( dest );
		str = QString( dest );
	}
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
// Get the raw headpose, so it can be displayed.
//
void Tracker::getHeadPose( THeadPoseData *data ) {
	data->x = Tracker::X.headPos;				// centimeters
	data->y = Tracker::Y.headPos;
	data->z = Tracker::Z.headPos;

	data->pitch = Tracker::Pitch.headPos;		// degrees
	data->yaw = Tracker::Yaw.headPos;
	data->roll = Tracker::Roll.headPos;
}

//
// Get the output-headpose, so it can be displayed.
//
void Tracker::getOutputHeadPose( THeadPoseData *data ) {
	data->x = output_camera.x;										// centimeters
	data->y = output_camera.y;
	data->z = output_camera.z;

	data->pitch = output_camera.pitch;	// degrees
	data->yaw   = output_camera.yaw;
	data->roll  = output_camera.roll;
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
// Load the current Settings from the currently 'active' INI-file.
//
void Tracker::loadSettings() {
//int NeutralZone;
//int sensYaw, sensPitch, sensRoll;
//int sensX, sensY, sensZ;

	qDebug() << "Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

	//
	// Read the Tracking settings, to fill the curves.
	//
	//iniFile.beginGroup ( "Tracking" );
	//NeutralZone = iniFile.value ( "NeutralZone", 5 ).toInt();
	//sensYaw = iniFile.value ( "sensYaw", 100 ).toInt();
	//sensPitch = iniFile.value ( "sensPitch", 100 ).toInt();
	//sensRoll = iniFile.value ( "sensRoll", 100 ).toInt();
	//sensX = iniFile.value ( "sensX", 100 ).toInt();
	//sensY = iniFile.value ( "sensY", 100 ).toInt();
	//sensZ = iniFile.value ( "sensZ", 100 ).toInt();
	//iniFile.endGroup ();

	//
	// Read the keyboard shortcuts.
	//
	iniFile.beginGroup ( "KB_Shortcuts" );
	
	// Center key
	CenterMouseKey = iniFile.value ( "MouseKey_Center", 0 ).toInt();
	CenterKey.keycode = iniFile.value ( "Keycode_Center", DIK_HOME ).toInt();
	CenterKey.shift = iniFile.value ( "Shift_Center", 0 ).toBool();
	CenterKey.ctrl = iniFile.value ( "Ctrl_Center", 0 ).toBool();
	CenterKey.alt = iniFile.value ( "Alt_Center", 0 ).toBool();
	DisableBeep = iniFile.value ( "Disable_Beep", 0 ).toBool();

	// StartStop key
	StartStopMouseKey = iniFile.value ( "MouseKey_StartStop", 0 ).toInt();
	StartStopKey.keycode = iniFile.value ( "Keycode_StartStop", DIK_END ).toInt();
	StartStopKey.shift = iniFile.value ( "Shift_StartStop", 0 ).toBool();
	StartStopKey.ctrl = iniFile.value ( "Ctrl_StartStop", 0 ).toBool();
	StartStopKey.alt = iniFile.value ( "Alt_StartStop", 0 ).toBool();
	setZero = iniFile.value ( "SetZero", 1 ).toBool();
	setEngineStop = iniFile.value ( "SetEngineStop", 1 ).toBool();

	// Inhibit key
	InhibitMouseKey = iniFile.value ( "MouseKey_Inhibit", 0 ).toInt();
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

	// Game Zero key
	GameZeroMouseKey = iniFile.value ( "MouseKey_GameZero", 0 ).toInt();
	GameZeroKey.keycode = iniFile.value ( "Keycode_GameZero", 0 ).toInt();
	GameZeroKey.shift = iniFile.value ( "Shift_GameZero", 0 ).toBool();
	GameZeroKey.ctrl = iniFile.value ( "Ctrl_GameZero", 0 ).toBool();
	GameZeroKey.alt = iniFile.value ( "Alt_GameZero", 0 ).toBool();

	// Axis Reverse key
	//AxisReverseKey.keycode = DIK_R;
	//AxisReverseKey.shift = false;
	//AxisReverseKey.ctrl = false;
	//AxisReverseKey.alt = false;

	// Reverse Axis
	useAxisReverse = iniFile.value ( "Enable_ReverseAxis", 0 ).toBool();
	YawAngle4ReverseAxis = iniFile.value ( "RA_Yaw", 40 ).toInt();
	Z_Pos4ReverseAxis = iniFile.value ( "RA_ZPos", 50 ).toInt();
	Z_PosWhenReverseAxis = iniFile.value ( "RA_ToZPos", 80 ).toInt();

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

//
// Determine if the MouseKey is pressed.
//
bool Tracker::isMouseKeyPressed( int *key, DIMOUSESTATE *mousestate ){

	//
	// If key == NONE, or invalid: ready!
	//
	if ((*key <= 0) || (*key > 5)) {
		return false;
	}

	//
	// Now, check if the right key is pressed.
	//
	if (mousestate->rgbButtons[*key-1] & 0x80) {
		return true;
	}
	else {
		return false;
	}
}
