/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay for				*
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

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

// Flags
bool Tracker::confid = false;
bool Tracker::set_initial = false;
bool Tracker::do_tracking = true;
bool Tracker::do_center = false;
bool Tracker::useFilter = false;

long Tracker::prevHeadPoseTime = 0;
THeadPoseDOF Tracker::Pitch;							// One structure for each of 6DOF's
THeadPoseDOF Tracker::Yaw;
THeadPoseDOF Tracker::Roll;
THeadPoseDOF Tracker::X;
THeadPoseDOF Tracker::Y;
THeadPoseDOF Tracker::Z;

TShortKey Tracker::CenterKey;							// ShortKey to Center headposition
TShortKey Tracker::StartStopKey;						// ShortKey to Start/stop tracking

/** constructor **/
Tracker::Tracker( int clientID ) {

	// Remember the selected client, from the ListBox
	// If the Tracker runs, this can NOT be changed...
	selectedClient = (FTNoIR_Client) clientID;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

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

	//
	// Initialize all server-handles. Only start the server, that was selected in the GUI.
	//
	server_FT = 0;
	server_FG = 0;
	server_PPJoy = 0;
	server_FTIR = 0;
	server_SC = 0;
	server_FSUIPC = 0;
	switch (selectedClient) {
		case FREE_TRACK:
			server_FT = new FTServer;					// Create Free-track protocol-server
			break;

		case FLIGHTGEAR:
			server_FG = new FGServer ( this );			// Create FlightGear protocol-server
			break;

		case FTNOIR:
			break;

		case PPJOY:
			server_PPJoy = new PPJoyServer ( this );	// Create PPJoy protocol-server
			break;

		case TRACKIR:
			server_FTIR = new FTIRServer;				// Create Fake-TIR protocol-server
			break;

		case SIMCONNECT:
			server_SC = new SCServer;					// Create SimConnect protocol-server
			break;

		case FSUIPC:
			server_FSUIPC = new FSUIPCServer;			// Create FSUIPC protocol-server
			break;

		default:
			// should never be reached
		break;
	}
	// Load the settings from the INI-file
	loadSettings();
}

/** destructor empty **/
Tracker::~Tracker() {

	// Stop the started server(s)
	if (server_FT) {
		server_FT->deleteLater();
	}
	if (server_FG) {
		server_FG->deleteLater();
	}
	if (server_PPJoy) {
		server_PPJoy->deleteLater();
	}
	if (server_FTIR) {
		server_FTIR->deleteLater();
	}
	if (server_SC) {
		server_SC->deleteLater();
	}

	if (server_FSUIPC) {
		server_FSUIPC->deleteLater();
	}

	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	::WaitForSingleObject(m_WaitThread, INFINITE);

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

	_engine->stop();
	smAPIQuit();

}

/** setting up the tracker engine **/
void Tracker::setup(QWidget *head, FaceTrackNoIR *parent) {
	bool DLL_Ok;

	// retrieve pointers to the User Interface and the main Application
	headPoseWidget = head;
	mainApp = parent;

	//registers the faceapi callback for receiving headpose data **/
	registerHeadPoseCallback();

	// some parameteres [optional]
	smHTSetHeadPosePredictionEnabled( _engine->handle(), false);
	smHTSetLipTrackingEnabled( _engine->handle(), false);
	smLoggingSetFileOutputEnable( false );

	// set up the line edits for calling
	headXLine = headPoseWidget->findChild<QLineEdit *>("headXLine");
	headYLine = headPoseWidget->findChild<QLineEdit *>("headYLine");
	headZLine = headPoseWidget->findChild<QLineEdit *>("headZLine");

	headRotXLine = headPoseWidget->findChild<QLineEdit *>("headRotXLine");
	headRotYLine = headPoseWidget->findChild<QLineEdit *>("headRotYLine");
	headRotZLine = headPoseWidget->findChild<QLineEdit *>("headRotZLine");


	//
	// Check if the Freetrack Client DLL is available
	// and create the necessary mapping to shared memory.
	// The handle of the MainWindow is sent to 'The Game', so it can send a message back.
	//
	if (server_FT) {
		DLL_Ok = server_FT->FTCheckClientDLL();
		DLL_Ok = server_FT->FTCreateMapping( mainApp->winId() );

		server_FT->start();								// Start the thread
	}

	// FlightGear
	if (server_FG) {
		server_FG->start();								// Start the thread
	}

	// PPJoy virtual joystick
	if (server_PPJoy) {
		server_PPJoy->start();							// Start the thread
	}

	//
	// Check if the NP Client DLL is available
	// and create the necessary mapping to shared memory.
	// The handle of the MainWindow is sent to 'The Game', so it can send a message back.
	//
	if (server_FTIR) {
		DLL_Ok = server_FTIR->FTIRCheckClientDLL();
		DLL_Ok = server_FTIR->FTIRCreateMapping( mainApp->winId() );

		server_FTIR->start();								// Start the thread
	}
	//
	// Check if the SimConnect DLL is available, load it if so.
	//
	if (server_SC) {
		DLL_Ok = server_SC->SCCheckClientDLL();

		if (DLL_Ok) {
			server_SC->start();									// Start the thread
		}
		else {
			QMessageBox::information(mainApp, "FaceTrackNoIR error", "SimConnect is not (correctly) installed!");
		}
	}

	//
	// Check if the FSUIPC DLL is available, load it if so.
	//
	if (server_FSUIPC) {
		DLL_Ok = server_FSUIPC->CheckClientDLL();

		if (DLL_Ok) {
			server_FSUIPC->start();									// Start the thread
		}
		else {
			QMessageBox::information(mainApp, "FaceTrackNoIR error", "FSUIPC is not (correctly) installed!");
		}
	}

}

/** QThread run method @override **/
void Tracker::run() {
	/** Direct Input variables **/
	LPDIRECTINPUT8 din;								// the pointer to our DirectInput interface
	LPDIRECTINPUTDEVICE8 dinkeyboard;				// the pointer to the keyboard device
	BYTE keystate[256];								// the storage for the key-information
	HRESULT retAcquire;
	bool lastBackKey = false;						// Remember state, to detect rising edge
	bool lastEqualsKey = false;

	float rawrotX, rawrotY, rawrotZ;				// Locals...
	float rawposX, rawposY, rawposZ;
	float rotX, rotY, rotZ;							// Locals...
	float posX, posY, posZ;

	SYSTEMTIME now;
	long newHeadPoseTime;
	float dT;

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
				// Check the state of the BACK key (= Start/Stop tracking) and EQUALS key (= Center)
				//
				if ( isShortKeyPressed( &CenterKey, &keystate[0] ) ) {
				   qDebug() << "Tracker::run Shortkey Center pressed!" << GetLastError();
				}
				//
				// Check the state of the BACK key (= Start/Stop tracking) and EQUALS key (= Center)
				//
				if ( isShortKeyPressed( &StartStopKey, &keystate[0] ) && (!lastBackKey) ) {
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
				lastBackKey = isShortKeyPressed( &StartStopKey, &keystate[0] );		// Remember

				if ( isShortKeyPressed( &CenterKey, &keystate[0] ) && (!lastEqualsKey) ) {
					Tracker::do_center = true;
					qDebug() << "Tracker::run() says Center pressed";
				}
				lastEqualsKey = isShortKeyPressed( &CenterKey, &keystate[0] );		// Remember
		   }
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

//////		Use this for some debug-output to file...
////		QFile data("output.txt");
////		if (data.open(QFile::WriteOnly | QFile::Append)) {
////			QTextStream out(&data);
////				out << Pitch.NeutralZone << " " << getDegreesFromRads(rotX) << " " << getOutputFromCurve(&Pitch.curve, getDegreesFromRads(rotX), Pitch.NeutralZone, Pitch.MaxInput)  << '\n';
//////			out << dT << " " << getSmoothFromList( &Pitch.rawList ) << " " << Pitch.offset_headPos << '\n';
////		}

			// Pitch
			if (Tracker::useFilter) {
				rotX = lowPassFilter ( getSmoothFromList( &Pitch.rawList ) - Pitch.offset_headPos - Pitch.initial_headPos, 
											   &Pitch.prevPos, dT, Tracker::Pitch.red );
			}
			else {
				rotX = getSmoothFromList( &Pitch.rawList ) - Pitch.offset_headPos - Pitch.initial_headPos;
			}
			rotX = Pitch.invert * getOutputFromCurve(&Pitch.curve, rotX, Pitch.NeutralZone, Pitch.MaxInput);

			// Yaw
			if (Tracker::useFilter) {
				rotY = lowPassFilter ( getSmoothFromList( &Yaw.rawList ) - Yaw.offset_headPos - Yaw.initial_headPos, 
											   &Yaw.prevPos, dT, Tracker::Yaw.red );
			}
			else {
				rotY = getSmoothFromList( &Yaw.rawList ) - Yaw.offset_headPos - Yaw.initial_headPos;
			}
			rotY = Yaw.invert * getOutputFromCurve(&Yaw.curve, rotY, Yaw.NeutralZone, Yaw.MaxInput);

			// Roll
			if (Tracker::useFilter) {
				rotZ = lowPassFilter ( getSmoothFromList( &Roll.rawList ) - Roll.offset_headPos - Roll.initial_headPos, 
											   &Roll.prevPos, dT, Tracker::Roll.red );
			}
			else {
				rotZ = getSmoothFromList( &Roll.rawList ) - Roll.offset_headPos - Roll.initial_headPos;
			}
			rotZ = Roll.invert * getOutputFromCurve(&Roll.curve, rotZ, Roll.NeutralZone, Roll.MaxInput);

			// X
			if (Tracker::useFilter) {
				posX = lowPassFilter ( getSmoothFromList( &X.rawList ) - X.offset_headPos - X.initial_headPos, 
											   &X.prevPos, dT, Tracker::X.red );
			}
			else {
				posX = getSmoothFromList( &X.rawList ) - X.offset_headPos - X.initial_headPos;
			}
			posX = X.invert * getOutputFromCurve(&X.curve, posX, X.NeutralZone, X.MaxInput);

			// Y
			if (Tracker::useFilter) {
				posY = lowPassFilter ( getSmoothFromList( &Y.rawList ) - Y.offset_headPos - Y.initial_headPos, 
											   &Y.prevPos, dT, Tracker::Y.red );
			}
			else {
				posY = getSmoothFromList( &Y.rawList ) - Y.offset_headPos - Y.initial_headPos;
			}
			posY = Y.invert * getOutputFromCurve(&Y.curve, posY, Y.NeutralZone, Y.MaxInput);

			// Z
			if (Tracker::useFilter) {
				posZ = lowPassFilter ( getSmoothFromList( &Z.rawList ) - Z.offset_headPos - Z.initial_headPos, 
											   &Z.prevPos, dT, Tracker::Z.red );
			}
			else {
				posZ = getSmoothFromList( &Z.rawList ) - Z.offset_headPos - Z.initial_headPos;
			}
			posZ = Z.invert * getOutputFromCurve(&Z.curve, posZ, Z.NeutralZone, Z.MaxInput);


			//
			// Send the Virtual Pose to selected Protocol-Server
			//
			// Free-track
			if (server_FT) {
				server_FT->setVirtRotX ( rotX );				// degrees
				server_FT->setVirtRotY ( rotY );
				server_FT->setVirtRotZ ( rotZ );

				server_FT->setVirtPosX ( posX );				// centimeters
				server_FT->setVirtPosY ( posY );
				server_FT->setVirtPosZ ( posZ );

				server_FT->setHeadRotX( rotX );					// degrees
				server_FT->setHeadRotY( rotY );
				server_FT->setHeadRotZ( rotZ );

				server_FT->setHeadPosX( posX );					// centimeters
				server_FT->setHeadPosY( posY );
				server_FT->setHeadPosZ( posZ );

			}

			// FlightGear
			if (server_FG) {
				server_FG->setVirtRotX ( rotX );				// degrees
				server_FG->setVirtRotY ( rotY );
				server_FG->setVirtRotZ ( rotZ );
				server_FG->setVirtPosX ( posX );				// centimeters
				server_FG->setVirtPosY ( posY );
				server_FG->setVirtPosZ ( posZ );
			}

			// PPJoy virtual joystick
			if (server_PPJoy) {
				server_PPJoy->setVirtRotX ( rotX );				// degrees
				server_PPJoy->setVirtRotY ( rotY );
				server_PPJoy->setVirtRotZ ( rotZ );

				server_PPJoy->setVirtPosX ( posX );				// centimeters
				server_PPJoy->setVirtPosY ( posY );
				server_PPJoy->setVirtPosZ ( posZ );
			}

			// Fake-trackIR
			if (server_FTIR) {
				server_FTIR->setVirtRotX ( rotX );				// degrees
				server_FTIR->setVirtRotY ( rotY );
				server_FTIR->setVirtRotZ ( rotZ );

				server_FTIR->setVirtPosX ( posX );				// centimeters
				server_FTIR->setVirtPosY ( posY );
				server_FTIR->setVirtPosZ ( posZ );
			}

			// SimConnect
			if (server_SC) {
				server_SC->setVirtRotX ( rotX );				// degrees
				server_SC->setVirtRotY ( rotY );
				server_SC->setVirtRotZ ( rotZ );

				server_SC->setVirtPosX ( posX );				// centimeters
				server_SC->setVirtPosY ( posY );
				server_SC->setVirtPosZ ( posZ );
			}

			// FSUIPC
			if (server_FSUIPC) {
				server_FSUIPC->setVirtRotX ( rotX );				// degrees
				server_FSUIPC->setVirtRotY ( rotY );
				server_FSUIPC->setVirtRotZ ( rotZ );

				server_FSUIPC->setVirtPosX ( posX );				// centimeters
				server_FSUIPC->setVirtPosY ( posY );
				server_FSUIPC->setVirtPosZ ( posZ );
			}

		}
		else {
			//
			// Go to initial position
			//
			if (server_FT) {
				server_FT->setVirtRotX ( 0.0f );
				server_FT->setVirtRotY ( 0.0f );
				server_FT->setVirtRotZ ( 0.0f );
				server_FT->setVirtPosX ( 0.0f );
				server_FT->setVirtPosY ( 0.0f );
				server_FT->setVirtPosZ ( 0.0f );
			}

			if (server_FG) {
				server_FG->setVirtRotX ( 0.0f );
				server_FG->setVirtRotY ( 0.0f );
				server_FG->setVirtRotZ ( 0.0f );
				server_FG->setVirtPosX ( 0.0f );
				server_FG->setVirtPosY ( 0.0f );
				server_FG->setVirtPosZ ( 0.0f );
			}

			if (server_PPJoy) {
				server_PPJoy->setVirtRotX ( 0.0f );
				server_PPJoy->setVirtRotY ( 0.0f );
				server_PPJoy->setVirtRotZ ( 0.0f );
				server_PPJoy->setVirtPosX ( 0.0f );
				server_PPJoy->setVirtPosY ( 0.0f );
				server_PPJoy->setVirtPosZ ( 0.0f );
			}

			if (server_FTIR) {
				server_FTIR->setVirtRotX ( 0.0f );
				server_FTIR->setVirtRotY ( 0.0f );
				server_FTIR->setVirtRotZ ( 0.0f );
				server_FTIR->setVirtPosX ( 0.0f );
				server_FTIR->setVirtPosY ( 0.0f );
				server_FTIR->setVirtPosZ ( 0.0f );
			}

			if (server_SC) {
				server_SC->setVirtRotX ( 0.0f );
				server_SC->setVirtRotY ( 0.0f );
				server_SC->setVirtRotZ ( 0.0f );
				server_SC->setVirtPosX ( 0.0f );
				server_SC->setVirtPosY ( 0.0f );
				server_SC->setVirtPosZ ( 0.0f );
			}

			if (server_FSUIPC) {
				server_FSUIPC->setVirtRotX ( 0.0f );
				server_FSUIPC->setVirtRotY ( 0.0f );
				server_FSUIPC->setVirtRotZ ( 0.0f );
				server_FSUIPC->setVirtPosX ( 0.0f );
				server_FSUIPC->setVirtPosY ( 0.0f );
				server_FSUIPC->setVirtPosZ ( 0.0f );
			}
		}

		//for lower cpu load 
		msleep(15);
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
	// Write the Raw headpose-data and add it to the RawList, for processing...
	//
	if( head_pose.confidence > 0 ) {
		Tracker::confid = true;

		// Pitch
		Tracker::Pitch.headPos = head_pose.head_rot.x_rads * 57.295781f;			// degrees
		addRaw2List ( &Pitch.rawList, Pitch.maxItems, Tracker::Pitch.headPos );

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

	} else {
		Tracker::confid = false;
	}

	// for lower cpu load
	msleep(15);
	yieldCurrentThread(); 
}

//
// Get the ProgramName from the Game and return it.
//
QString Tracker::getGameProgramName() {
QString str;

	if ( server_FT ) {
		str = server_FT->FTGetProgramName();
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
			if ( _engine ) {
				_engine->stop();
				Tracker::set_initial = false;
				_engine->start();
			}
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
// All previou values are taken into account, the weight of this is determined by 'coeff'.
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
