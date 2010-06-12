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
*																				*
* PPJoyServer		PPJoyServer is the Class, that communicates headpose-data	*
*					to the Virtual Joystick, created by Deon van der Westhuysen.*
********************************************************************************/
#include <QtGui>
#include <QtNetwork>
#include "PPJoyServer.h"
#include "Tracker.h"
#include <Winsock.h>

long PPJoyServer::PPJoyCorrection = 1470;
long PPJoyServer::analogDefault = (PPJOY_AXIS_MIN+PPJOY_AXIS_MAX)/2 - PPJoyServer::PPJoyCorrection;
static const char* DevName = "\\\\.\\PPJoyIOCTL1";

/** constructor **/
PPJoyServer::PPJoyServer( Tracker *parent ) {

	// Save the parent
	headTracker = parent;

	// Create events
	m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
	m_WaitThread = CreateEvent(0, TRUE, FALSE, 0);

	// Initialize arrays
	for (int i = 0;i < 3;i++) {
		centerPos[i] = 0;
		centerRot[i] = 0;
	}

	/* Open a handle to the control device for the first virtual joystick. */
	/* Virtual joystick devices are names PPJoyIOCTL1 to PPJoyIOCTL16. */
	h = CreateFileA((LPCSTR) DevName,GENERIC_WRITE,FILE_SHARE_WRITE,NULL,OPEN_EXISTING,0,NULL);

	/* Make sure we could open the device! */
	if (h == INVALID_HANDLE_VALUE)
	{
		QMessageBox::critical(0, "Connection Failed", QString("FaceTrackNoIR failed to connect to Virtual Joystick.\nCheck if it was properly installed!"));
		return;
	}
}

/** destructor **/
PPJoyServer::~PPJoyServer() {

	// Trigger thread to stop
	::SetEvent(m_StopThread);

	// Wait until thread finished
	::WaitForSingleObject(m_WaitThread, INFINITE);

	// Close handles
	::CloseHandle(m_StopThread);
	::CloseHandle(m_WaitThread);

	//
	// Free the Virtual Joystick
	//
	CloseHandle(h);
	//terminates the QThread and waits for finishing the QThread
	terminate();
	wait();
}

/** QThread run @override **/
void PPJoyServer::run() {


	/* Initialise the IOCTL data structure */
	JoyState.Signature= JOYSTICK_STATE_V1;
	JoyState.NumAnalog= NUM_ANALOG;					// Number of analog values
	Analog= JoyState.Analog;						// Keep a pointer to the analog array for easy updating
	Digital= JoyState.Digital;						// Keep a pointer to the digital array for easy updating
	JoyState.NumDigital= NUM_DIGITAL;				// Number of digital values

	/* Make sure we could open the device! */
	/* MessageBox in run() does not work! (runtime error...)*/
	if (h == INVALID_HANDLE_VALUE) {
		return;
	}

	forever
	{
	    // Check event for stop thread
		if(::WaitForSingleObject(m_StopThread, 0) == WAIT_OBJECT_0) {
			// Set event
			::SetEvent(m_WaitThread);
			return;
		}

		// The effective angle for faceTracking will be < 90 degrees, so we assume a smaller range here
		Analog[0] = scale2AnalogLimits( virtRotX, -50.0f, 50.0f );	// Pitch
		qDebug() << "PPJoyServer says: Pitch =" << Analog[0] << " VirtRotX =" << virtRotX ;
		Analog[1] = scale2AnalogLimits( virtRotY, -50.0f, 50.0f );	// Yaw
		Analog[2] = scale2AnalogLimits( virtRotZ, -50.0f, 50.0f );	// Roll

		// The effective movement for faceTracking will be < 50 cm, so we assume a smaller range here
		Analog[3] = scale2AnalogLimits( virtPosX, -40.0f, 40.0f );						// X

		Analog[5] = scale2AnalogLimits( virtPosY, -40.0f, 40.0f );						// Y (5?)
		Analog[6] = scale2AnalogLimits( virtPosZ, -40.0f, 40.0f );						// Z (6?)

		checkAnalogLimits();

		/* Send request to PPJoy for processing. */
		/* Currently there is no Return Code from PPJoy, this may be added at a */
		/* later stage. So we pass a 0 byte output buffer.                      */
		if (!DeviceIoControl( h, IOCTL_PPORTJOY_SET_STATE, &JoyState, sizeof(JoyState), NULL, 0, &RetSize, NULL))
		{
			return;
		}
		// just for lower cpu load
		msleep(15);	
		yieldCurrentThread();
	}
}

//
// Limit the Joystick values
//
void PPJoyServer::checkAnalogLimits() {
	for (int i = 0;i < NUM_ANALOG;i++) {
		if (Analog[i]>PPJOY_AXIS_MAX) {
			Analog[i]=PPJOY_AXIS_MAX;
		}
		else if (Analog[i]<PPJOY_AXIS_MIN) {
			Analog[i]=PPJOY_AXIS_MIN;
		}
	}
}

//
// Scale the measured value to the Joystick values
//
long PPJoyServer::scale2AnalogLimits( float x, float min_x, float max_x ) {
double y;

	y = ((PPJOY_AXIS_MAX - PPJOY_AXIS_MIN)/(max_x - min_x)) * x + ((PPJOY_AXIS_MAX - PPJOY_AXIS_MIN)/2) + PPJOY_AXIS_MIN;
	qDebug() << "scale2AnalogLimits says: long_y =" << y;

	return (long) y;
}
//END
