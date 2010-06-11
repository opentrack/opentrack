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

float Tracker::rotNeutralZone = 0.087f;					// Neutral Zone for rotations (rad)
long Tracker::prevHeadPoseTime = 0;
THeadPoseDOF Tracker::Pitch;							// One structure for each of 6DOF's
THeadPoseDOF Tracker::Yaw;
THeadPoseDOF Tracker::Roll;
THeadPoseDOF Tracker::X;
THeadPoseDOF Tracker::Y;
THeadPoseDOF Tracker::Z;

/** constructor empty **/
Tracker::Tracker() {

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

	server_FT = new FTServer;							// Create the new thread (on the heap)
	server_FG = new FGServer ( this );					// Create the new thread (on the heap)
}

/** destructor empty **/
Tracker::~Tracker() {

	server_FT->deleteLater();
	server_FG->deleteLater();

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
	DLL_Ok = server_FT->FTCheckClientDLL();
	DLL_Ok = server_FT->FTCreateMapping( mainApp->winId() );

	qDebug() << "FaceTrackNoIR says: Window Handle =" << mainApp->winId();

//	return;
	server_FT->start();									// Should start at the push of a button?
	server_FG->start();									// 
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

	SYSTEMTIME now;
	long newHeadPoseTime;
	float dT;

	//QFile data("output.txt");
	//if (data.open(QFile::WriteOnly | QFile::Truncate)) {
	//	QTextStream out(&data);
	//	out << "Polling results";
	//}

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
				if ( (keystate[DIK_BACK] & 0x80) && (!lastBackKey) ) {
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
					qDebug() << "Tracker::run() says BACK pressed, do_tracking =" << Tracker::do_tracking;
				}
				lastBackKey = (keystate[DIK_BACK] & 0x80);						// Remember

				if ( (keystate[DIK_EQUALS] & 0x80) && (!lastEqualsKey) ) {
					Tracker::do_center = true;
					qDebug() << "Tracker::run() says EQUALS pressed";
				}
				lastEqualsKey = (keystate[DIK_EQUALS] & 0x80);					// Remember
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
				Tracker::Z.initial_headPos = Tracker::getHeadPosZ();
				MessageBeep (MB_ICONASTERISK);
				Tracker::set_initial = true;
			}

			headXLine->setText(QString("%1").arg(Tracker::getHeadPosX()*100, 0, 'f', 1));
			headYLine->setText(QString("%1").arg(Tracker::getHeadPosY()*100, 0, 'f', 1));
			headZLine->setText(QString("%1").arg(Tracker::getHeadPosZ()*100, 0, 'f', 1));

			headRotXLine->setText(QString("%1").arg(Tracker::getHeadRotX()*100, 0, 'f', 1));
			headRotYLine->setText(QString("%1").arg(Tracker::getHeadRotY()*100, 0, 'f', 1));
			headRotZLine->setText(QString("%1").arg(Tracker::getHeadRotZ()*100, 0, 'f', 1));
////			listener.setTrackedPosition(QPoint(Tracker::getHeadPosX()-50, Tracker::getHeadPosY()-37.5));

			//
			// Copy the Raw values directly to Free-track server
			//
			server_FT->setHeadRotX( Tracker::Pitch.headPos );			// rads
			server_FT->setHeadRotY( Tracker::Yaw.headPos );
			server_FT->setHeadRotZ( Tracker::Roll.headPos);

			server_FT->setHeadPosX( Tracker::X.headPos * 1000.0f);		// From m to mm
			server_FT->setHeadPosY( Tracker::Y.headPos * 1000.0f);
			server_FT->setHeadPosZ( ( Tracker::Z.headPos - Tracker::Z.initial_headPos ) * 1000.0f);

		}

		//
		// If Center is pressed, copy the current values to the offsets.
		//
		if (Tracker::do_center && Tracker::set_initial) {
			Pitch.offset_headPos = getSmoothFromList( &Pitch.rawList );
			Yaw.offset_headPos = getSmoothFromList( &Yaw.rawList );
			Roll.offset_headPos = getSmoothFromList( &Roll.rawList );
			X.offset_headPos = getSmoothFromList( &X.rawList );
			Y.offset_headPos = getSmoothFromList( &Y.rawList );

			//
			// Reset the initial distance to the camera
			//
			Z.offset_headPos = getSmoothFromList( &Z.rawList ) - Tracker::Z.initial_headPos;
			Tracker::do_center = false;
		}

		if (Tracker::do_tracking && Tracker::confid) {
			//
			// Also send the Virtual Pose to FT-server and FG-server
			//
			if (Tracker::useFilter) {
				Pitch.newPos = lowPassFilter ( getSmoothFromList( &Pitch.rawList ) - Pitch.offset_headPos, 
											   &Pitch.prevPos, dT, Tracker::Pitch.red );
			}
			else {
				Pitch.newPos = getSmoothFromList( &Pitch.rawList ) - Pitch.offset_headPos;
			}
			server_FT->setVirtRotX ( Tracker::Pitch.invert * Tracker::Pitch.sens * Pitch.newPos );

			if (Tracker::useFilter) {
				Yaw.newPos = lowPassFilter ( getSmoothFromList( &Yaw.rawList ) - Yaw.offset_headPos, 
											   &Yaw.prevPos, dT, Tracker::Yaw.red );
			}
			else {
				Yaw.newPos = getSmoothFromList( &Yaw.rawList ) - Yaw.offset_headPos;
			}
			server_FT->setVirtRotY ( Tracker::Yaw.invert   * Tracker::Yaw.sens   *  Yaw.newPos );

			if (Tracker::useFilter) {
				Roll.newPos = lowPassFilter ( getSmoothFromList( &Roll.rawList ) - Roll.offset_headPos, 
											   &Roll.prevPos, dT, Tracker::Roll.red );
			}
			else {
				Roll.newPos = getSmoothFromList( &Roll.rawList ) - Roll.offset_headPos;
			}
			server_FT->setVirtRotZ ( Tracker::Roll.invert  * Tracker::Roll.sens  * Roll.newPos );

			server_FT->setVirtPosX ( ( Tracker::X.invert   * Tracker::X.sens     * (getSmoothFromList( &X.rawList ) - X.offset_headPos) ) * 1000.0f);
			server_FT->setVirtPosY ( ( Tracker::Y.invert   * Tracker::Y.sens     * (getSmoothFromList( &Y.rawList ) - Y.offset_headPos) ) * 1000.0f );
			server_FT->setVirtPosZ ( ( Tracker::Z.invert   * Tracker::Z.sens     * (getSmoothFromList( &Z.rawList ) - Z.offset_headPos - Tracker::Z.initial_headPos) ) * 1000.0f );

			server_FG->setVirtRotX ( getDegreesFromRads ( Tracker::Pitch.invert * Tracker::Pitch.sens * (getSmoothFromList( &Pitch.rawList ) - Pitch.offset_headPos) ) );
			server_FG->setVirtRotY ( getDegreesFromRads ( Tracker::Yaw.invert   * Tracker::Yaw.sens   * (getSmoothFromList( &Yaw.rawList )   - Yaw.offset_headPos) ) );
			server_FG->setVirtRotZ ( getDegreesFromRads ( Tracker::Roll.invert  * Tracker::Roll.sens  * (getSmoothFromList( &Roll.rawList )  - Roll.offset_headPos) ) );
			server_FG->setVirtPosX ( Tracker::X.invert * Tracker::X.sens * (getSmoothFromList( &X.rawList ) - X.offset_headPos) );
			server_FG->setVirtPosY ( Tracker::Y.invert * Tracker::Y.sens * (getSmoothFromList( &Y.rawList ) - Y.offset_headPos) );
			server_FG->setVirtPosZ ( Tracker::Z.invert * Tracker::Z.sens * (getSmoothFromList( &Z.rawList ) - Z.offset_headPos - Tracker::Z.initial_headPos) );
		}
		else {
			//
			// Go to initial position
			//
			server_FT->setVirtRotX ( 0.0f );
			server_FT->setVirtRotY ( 0.0f );
			server_FT->setVirtRotZ ( 0.0f );
			server_FT->setVirtPosX ( 0.0f );
			server_FT->setVirtPosY ( 0.0f );
			server_FT->setVirtPosZ ( 0.0f );

			server_FG->setVirtRotX ( 0.0f );
			server_FG->setVirtRotY ( 0.0f );
			server_FG->setVirtRotZ ( 0.0f );
			server_FG->setVirtPosX ( 0.0f );
			server_FG->setVirtPosY ( 0.0f );
			server_FG->setVirtPosZ ( 0.0f );
		}

		//for lower cpu load 
		msleep(25);
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
	//SYSTEMTIME now;
	//long newHeadPoseTime;
	//float dT;
//	float rate;

	//
	// Perform actions, when valid data is received from faceAPI.
	//
	if( head_pose.confidence > 0 ) {
		Tracker::confid = true;
		Tracker::setHeadPosX(head_pose.head_pos.x);
		Tracker::setHeadPosY(head_pose.head_pos.y);
		Tracker::setHeadPosZ(head_pose.head_pos.z);	

		Tracker::setHeadRotX(head_pose.head_rot.x_rads);
		Tracker::setHeadRotY(head_pose.head_rot.y_rads);
		Tracker::setHeadRotZ(head_pose.head_rot.z_rads);	

		////
		//// Get the System-time and substract the time from the previous call.
		//// dT will be used for the EWMA-filter.
		////
		//GetSystemTime ( &now );
		//newHeadPoseTime = (((now.wHour * 3600) + (now.wMinute * 60) + now.wSecond) * 1000) + now.wMilliseconds;
		//dT = (newHeadPoseTime - Tracker::prevHeadPoseTime) / 1000.0f;

		//// Remember time for next call
		//Tracker::prevHeadPoseTime = newHeadPoseTime;



		//
		// Calculate the new values, applying a low-pass filter.
		// Add the values to their respective QList, for further smoothing
		//
		// Pitch
		//if (Tracker::useFilter) {
		//	Pitch.newPos = lowPassFilter ( getCorrectedNewRaw ( rateLimiter ( Tracker::Pitch.headPos, &Tracker::Pitch.prevRawPos, dT, 1.0f ), rotNeutralZone ), 
		//		                           &Pitch.prevPos, dT, Tracker::Pitch.red );
		//}
		//else {
			Pitch.newPos = getCorrectedNewRaw ( Tracker::Pitch.headPos, rotNeutralZone );
		//}
		addRaw2List ( &Pitch.rawList, Pitch.maxItems, Pitch.newPos );

		//
		// Log something
		//
		//rate = rateLimiter ( Tracker::Pitch.headPos, &Tracker::Pitch.prevRawPos, dT, 1.0f );
		//QFile data("output.txt");
		//if (data.open(QFile::WriteOnly | QFile::Append)) {
		//	QTextStream out(&data);
		//	out << "Limited Raw= " << rate << " dT= " << dT << " Raw= " << Tracker::Pitch.headPos << " Filtered= " << Pitch.newPos << '\n';
		//}
//		Tracker::Pitch.prevRawPos = Tracker::Pitch.headPos;


		// Yaw
		//if (Tracker::useFilter) {
		//	Yaw.newPos = lowPassFilter ( getCorrectedNewRaw ( rateLimiter ( Tracker::Yaw.headPos, &Tracker::Yaw.prevRawPos, dT, 1.0f ), rotNeutralZone ), 
		//								 &Yaw.prevPos, dT, Tracker::Yaw.red );
		//}
		//else {
			Yaw.newPos = getCorrectedNewRaw ( Tracker::Yaw.headPos, rotNeutralZone );
		//}
		addRaw2List ( &Yaw.rawList, Yaw.maxItems, Yaw.newPos );

		// Roll
		//if (Tracker::useFilter) {
		//	Roll.newPos = lowPassFilter ( getCorrectedNewRaw (rateLimiter ( Tracker::Roll.headPos, &Tracker::Roll.prevRawPos, dT, 1.0f ), rotNeutralZone ),
		//								  &Roll.prevPos, dT, Tracker::Roll.red );
		//}
		//else {
			Roll.newPos = getCorrectedNewRaw ( Tracker::Roll.headPos, rotNeutralZone );
		//}
		addRaw2List ( &Roll.rawList, Roll.maxItems, Roll.newPos );

		// X-position
		//if (Tracker::useFilter) {
		//	X.newPos = lowPassFilter ( Tracker::X.headPos, &X.prevPos, dT, Tracker::X.red );
		//}
		//else {
			X.newPos = Tracker::X.headPos;
		//}
		addRaw2List ( &X.rawList, X.maxItems, X.newPos );

		// Y-position
		//if (Tracker::useFilter) {
		//	Y.newPos = lowPassFilter ( Tracker::Y.headPos, &Y.prevPos, dT, Tracker::Y.red );
		//}
		//else {
			Y.newPos = Tracker::Y.headPos;
		//}
		addRaw2List ( &Y.rawList, Y.maxItems, Y.newPos );

		// Z-position (distance to camera, absolute!)
		//if (Tracker::useFilter) {
		//	Z.newPos = lowPassFilter ( Tracker::Z.headPos, &Z.prevPos, dT, Tracker::Z.red );
		//}
		//else {
			Z.newPos = Tracker::Z.headPos;
		//}
		addRaw2List ( &Z.rawList, Z.maxItems, Z.newPos );

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