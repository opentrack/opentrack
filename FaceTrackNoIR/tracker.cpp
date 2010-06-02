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

float Tracker::headPosX = 0.0f;
float Tracker::headPosY = 0.0f;
float Tracker::headPosZ = 0.0f;
float Tracker::initial_headPosZ = 0.0f;

float Tracker::headRotX = 0.0f;
float Tracker::headRotY = 0.0f;
float Tracker::headRotZ = 0.0f;

// Offsets, to center headpos while tracking
float Tracker::offset_headPosX = 0.0f;
float Tracker::offset_headPosY = 0.0f;
float Tracker::offset_headPosZ = 0.0f;

float Tracker::offset_headRotX = 0.0f;
float Tracker::offset_headRotY = 0.0f;
float Tracker::offset_headRotZ = 0.0f;

// Flags
bool Tracker::confid = false;
bool Tracker::newdata = false;
bool Tracker::set_initial = false;
bool Tracker::do_tracking = true;
bool Tracker::do_center = false;

float Tracker::sensYaw = 1.0f;
float Tracker::sensPitch = 1.0f;
float Tracker::sensRoll = 1.0f;
float Tracker::sensX = 1.0f;
float Tracker::sensY = 1.0f;
float Tracker::sensZ = 1.0f;

float Tracker::invertYaw = 1.0f;
float Tracker::invertPitch = 1.0f;
float Tracker::invertRoll = 1.0f;
float Tracker::invertX = 1.0f;
float Tracker::invertY = 1.0f;
float Tracker::invertZ = 1.0f;

bool Tracker::useFilter = false;

float Tracker::redYaw = 0.7f;
float Tracker::redPitch = 0.7f;
float Tracker::redRoll = 0.7f;
float Tracker::redX = 0.7f;
float Tracker::redY = 0.7f;
float Tracker::redZ = 0.7f;

float Tracker::rotNeutralZone = 0.087f;					// Neutral Zone for rotations (rad)

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
	smHTSetHeadPosePredictionEnabled( _engine->handle(), true);
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
	bool lastMinusKey = false;						// Remember state, to detect rising edge
	bool lastEqualsKey = false;

	float prevYaw = 0.0f;							// Remember previous Raw, to filter jitter
	float prevPitch = 0.0f;
	float prevRoll = 0.0f;
	float prevX = 0.0f;
	float prevY = 0.0f;
	float prevZ = 0.0f;

	float newYaw = 0.0f;							// Local new Raw, to filter jitter
	float newPitch = 0.0f;
	float newRoll = 0.0f;
	float newX = 0.0f;
	float newY = 0.0f;
	float newZ = 0.0f;

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
				// Check the state of the MINUS key (= Start/Stop tracking) and EQUALS key (= Center)
				//
				if ( (keystate[DIK_BACK] & 0x80) && (!lastMinusKey) ) {
					Tracker::do_tracking = !Tracker::do_tracking;

					//
					// To start tracking again and be '0', execute Center command too
					//
					if (Tracker::do_tracking) {
						Tracker::do_center = true;
					}
					qDebug() << "Tracker::run() says BACK pressed, do_tracking =" << Tracker::do_tracking;
				}
				lastMinusKey = (keystate[DIK_BACK] & 0x80);					// Remember

				if ( (keystate[DIK_EQUALS] & 0x80) && (!lastEqualsKey) ) {
					Tracker::do_center = true;
					qDebug() << "Tracker::run() says EQUALS pressed";
				}
				lastEqualsKey = (keystate[DIK_EQUALS] & 0x80);					// Remember
		   }
		}

		//if the confidence is good enough the headpose will be updated **/
		if (Tracker::confid && Tracker::newdata) {

			//
			// Most games need an offset to the initial position and NOT the
			// absolute distance to the camera: so remember the initial distance
			// to substract that later...
			//
			if(Tracker::set_initial == false) {
				Tracker::initial_headPosZ = Tracker::getHeadPosZ();
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
			server_FT->setHeadRotX( Tracker::headRotX );				// rads
			server_FT->setHeadRotY( Tracker::headRotY );
			server_FT->setHeadRotZ( Tracker::headRotZ );

			server_FT->setHeadPosX( Tracker::headPosX * 1000.0f);		// From m to mm
			server_FT->setHeadPosY( Tracker::headPosY * 1000.0f);
			server_FT->setHeadPosZ( ( Tracker::headPosZ - Tracker::initial_headPosZ ) * 1000.0f);

			//
			// Calculate the new values, applying a low-pass filter.
			// Add the values to their respective QList, for further smoothing
			//
			if (Tracker::useFilter) {
				newPitch = lowPassFilter ( Tracker::headRotX, &prevPitch, 0.020f, Tracker::redPitch );
			}
			else {
				newPitch = Tracker::headRotX;
			}
			addRaw2List ( &rawPitchList, intMaxPitchItems, newPitch );
			//QTextStream out(&data);
			//out << "Raw:" << Tracker::headRotX << " filtered:" << newPitch << endl;
			if (Tracker::useFilter) {
				newYaw = lowPassFilter ( Tracker::headRotY, &prevYaw, 0.020f, Tracker::redYaw );
			}
			else {
				newYaw = Tracker::headRotY;
			}
			addRaw2List ( &rawYawList, intMaxYawItems, newYaw );
			if (Tracker::useFilter) {
				newRoll = lowPassFilter ( Tracker::headRotZ, &prevRoll, 0.020f, Tracker::redRoll );
			}
			else {
				newRoll = Tracker::headRotZ;
			}
			addRaw2List ( &rawRollList, intMaxRollItems, newRoll );
			if (Tracker::useFilter) {
				newX = lowPassFilter ( Tracker::headPosX, &prevX, 0.020f, Tracker::redX );
			}
			else {
				newX = Tracker::headPosX;
			}
			addRaw2List ( &rawXList, intMaxXItems, newX );
			if (Tracker::useFilter) {
				newY = lowPassFilter ( Tracker::headPosY, &prevY, 0.020f, Tracker::redY );
			}
			else {
				newY = Tracker::headPosY;
			}
			addRaw2List ( &rawYList, intMaxYItems, newY );
			if (Tracker::useFilter) {
				newZ = lowPassFilter ( Tracker::headPosZ, &prevZ, 0.020f, Tracker::redZ );
			}
			else {
				newZ = Tracker::headPosZ;
			}
			addRaw2List ( &rawZList, intMaxZItems, newZ );

			Tracker::newdata = false;								// Reset flag for ReceiveHeadPose
		}

		//
		// If Center is pressed, copy the current values to the offsets.
		//
		if (Tracker::do_center) {
			offset_headRotX = getSmoothFromList( &rawPitchList );
			offset_headRotY = getSmoothFromList( &rawYawList );
			offset_headRotZ = getSmoothFromList( &rawRollList );
			offset_headPosX = getSmoothFromList( &rawXList );
			offset_headPosY = getSmoothFromList( &rawYList );
			offset_headPosZ = getSmoothFromList( &rawZList );
			Tracker::do_center = false;
		}

		if (Tracker::do_tracking) {
			//
			// Also send the Virtual Pose to FT-server and FG-server
			//
			server_FT->setVirtRotX ( Tracker::invertPitch * Tracker::sensPitch * (getSmoothFromList( &rawPitchList ) - offset_headRotX) );
			server_FT->setVirtRotY ( Tracker::invertYaw   * Tracker::sensYaw   * (getSmoothFromList( &rawYawList ) - offset_headRotY) );
			server_FT->setVirtRotZ ( Tracker::invertRoll  * Tracker::sensRoll  * (getSmoothFromList( &rawRollList ) - offset_headRotZ) );
			server_FT->setVirtPosX ( ( Tracker::invertX   * Tracker::sensX     * (getSmoothFromList( &rawXList ) - offset_headPosX) ) * 1000.0f);
			server_FT->setVirtPosY ( ( Tracker::invertY   * Tracker::sensY     * (getSmoothFromList( &rawYList ) - offset_headPosY) ) * 1000.0f );
			server_FT->setVirtPosZ ( ( Tracker::invertZ   * Tracker::sensZ     * (getSmoothFromList( &rawZList ) - offset_headPosZ) ) * 1000.0f );

			server_FG->setVirtRotX ( getDegreesFromRads ( Tracker::invertPitch * Tracker::sensPitch * (getSmoothFromList( &rawPitchList ) - offset_headRotX) ) );
			server_FG->setVirtRotY ( getDegreesFromRads ( Tracker::invertYaw   * Tracker::sensYaw   * (getSmoothFromList( &rawYawList )   - offset_headRotY) ) );
			server_FG->setVirtRotZ ( getDegreesFromRads ( Tracker::invertRoll  * Tracker::sensRoll  * (getSmoothFromList( &rawRollList )  - offset_headRotZ) ) );
			server_FG->setVirtPosX ( Tracker::invertX * Tracker::sensX * (getSmoothFromList( &rawXList ) - offset_headPosX) );
			server_FG->setVirtPosY ( Tracker::invertY * Tracker::sensY * (getSmoothFromList( &rawYList ) - offset_headPosY) );
			server_FG->setVirtPosZ ( Tracker::invertZ * Tracker::sensZ * (getSmoothFromList( &rawZList ) - offset_headPosZ) );
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
		msleep(20);
		yieldCurrentThread(); 
	}
}

/** registers the faceapi headpose callback function **/
void Tracker::registerHeadPoseCallback() {
	Q_ASSERT(_engine_handle);
	smReturnCode error = smHTRegisterHeadPoseCallback( _engine->handle(), 0, receiveHeadPose);
	//showErrorBox(0, "Register HeadPose Callback", error);
	start(LowestPriority);
}

/** Callback function for head-pose - only static methods could be called **/
void Tracker::receiveHeadPose(void *,smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame)
{
	if(head_pose.confidence>0) {
		Tracker::confid = true;
		Tracker::setHeadPosX(head_pose.head_pos.x);
		Tracker::setHeadPosY(head_pose.head_pos.y);
		Tracker::setHeadPosZ(head_pose.head_pos.z);	

		Tracker::setHeadRotX(head_pose.head_rot.x_rads);
		Tracker::setHeadRotY(head_pose.head_rot.y_rads);
		Tracker::setHeadRotZ(head_pose.head_rot.z_rads);	
	} else {
		Tracker::confid = false;
	}
	Tracker::newdata = true;									// Set flag for run()
	// for lower cpu load
	msleep(40);
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
// Implementation of an Exponentially Weighed Moving Average, used to serve as a low-pass filter.
// The code was adopted from Melchior Franz, who created it for FlightGear (aircraft.nas).
//
// The function takes the new value, the delta-time (sec) and a weighing coefficient (>0 and <1)
//
float Tracker::lowPassFilter ( float newvalue, float *oldvalue, float dt, float coeff) {
float c = 0.0f;
float fil = 0.0f;

	c = dt / (coeff + dt);
	fil = (newvalue * c) + (*oldvalue * (1 - c));
	*oldvalue = fil;

	return fil;
}
