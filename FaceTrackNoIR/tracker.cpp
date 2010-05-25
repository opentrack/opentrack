/********************************************************************************
* SweetSpotter		This program is an research project of the Chair			*
*					of Communication Acoustics at TU Dresden, Germany.			*
*																				*
* Copyright (C) 2010	Lars Beier (Developing)									*
*						Sebastian Merchel (Researching)							*
*						Stephan Groth (Researching)								*
*																				*
* Homepage				<http://www.sweetspotter.de>							*
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
bool Tracker::confid = false;
bool Tracker::set_initial = false;

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

	// Let's start smoothing with 10 samples...
	intMaxYawItems = 10;
	intMaxPitchItems = 10;
	intMaxRollItems = 10;
	intMaxXItems = 10;
	intMaxYItems = 10;
	intMaxZItems = 10;

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

	forever
	{

	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0)
		{
			// Set event
			::SetEvent(m_WaitThread);
			return;
		}

		//if the confidence is good enough the headpose will be updated **/
		if(Tracker::confid) {

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

			server_FT->setHeadRotX( Tracker::headRotX );			// rads (?)
			server_FT->setHeadRotY( Tracker::headRotY );
			server_FT->setHeadRotZ( Tracker::headRotZ );

			server_FT->setHeadPosX( Tracker::headPosX * 1000.0f);	// From m to mm
			server_FT->setHeadPosY( Tracker::headPosY * 1000.0f);
			server_FT->setHeadPosZ( ( Tracker::headPosZ - Tracker::initial_headPosZ ) * 1000.0f);

			//
			// Add the raw values to the QList, so they can be smoothed.
			// The raw value that enters the QList is first (evt.) inverted and corrected for Neutral Zone.
			//
			addRaw2List ( &rawPitchList, intMaxPitchItems, getCorrectedNewRaw ( Tracker::invertPitch * Tracker::headRotX , Tracker::rotNeutralZone ) );
			addRaw2List ( &rawYawList, intMaxYawItems, getCorrectedNewRaw ( Tracker::invertYaw * Tracker::headRotY , Tracker::rotNeutralZone ) );
			addRaw2List ( &rawRollList, intMaxRollItems, getCorrectedNewRaw ( Tracker::invertRoll * Tracker::headRotZ , Tracker::rotNeutralZone ) );
			addRaw2List ( &rawXList, intMaxXItems, Tracker::invertX * Tracker::headPosX * 1000.0f );
			addRaw2List ( &rawYList, intMaxYItems, Tracker::invertY * Tracker::headPosY * 1000.0f );
			addRaw2List ( &rawZList, intMaxZItems, ( Tracker::invertZ * Tracker::headPosZ - Tracker::initial_headPosZ ) * 1000.0f );
		}

		//
		// Also send the Virtual Pose to FT-server
		//
		server_FT->setVirtRotX( Tracker::sensPitch * getSmoothFromList( &rawPitchList ) );
		server_FT->setVirtRotY( Tracker::sensYaw * getSmoothFromList( &rawYawList ) );
		server_FT->setVirtRotZ( Tracker::sensRoll * getSmoothFromList( &rawRollList ) );
		server_FT->setVirtPosX ( Tracker::sensX * getSmoothFromList( &rawXList ) );
		server_FT->setVirtPosY ( Tracker::sensY * getSmoothFromList( &rawYList ) );
		server_FT->setVirtPosZ ( Tracker::sensZ * getSmoothFromList( &rawZList ) );

		server_FG->setVirtRotX( getDegreesFromRads ( Tracker::sensPitch * getSmoothFromList( &rawPitchList ) ) );
		server_FG->setVirtRotY( getDegreesFromRads ( Tracker::sensYaw * getSmoothFromList( &rawYawList ) ) );
		server_FG->setVirtRotZ( getDegreesFromRads ( Tracker::sensRoll * getSmoothFromList( &rawRollList ) ) );
		server_FG->setVirtPosX ( ( Tracker::sensX * getSmoothFromList( &rawXList ) ) / 1000.0f );
		server_FG->setVirtPosY ( ( Tracker::sensY * getSmoothFromList( &rawYList ) ) / 1000.0f );
		server_FG->setVirtPosZ ( ( Tracker::sensZ * getSmoothFromList( &rawZList ) ) / 1000.0f );

		//for lower cpu load 
		msleep(50);
		yieldCurrentThread(); 
	}
}

/** registeres the faceapi headpose callback function **/
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
	// for lower cpu load
	msleep(50);
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
