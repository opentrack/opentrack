/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay for				*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*																				*
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
#ifndef __TRACKER_H__
#define __TRACKER_H__

#include <sm_api_qt.h>
#include <QThread>
#include <QMessageBox>
#include <QLineEdit>
#include <QThread>
#include <QPoint>
#include <QWaitCondition>
#include <QList>
#include <QPainterPath>

#define DIRECTINPUT_VERSION 0x0800
#include <Dinput.h>

#include "FTServer.h"				// Freetrack-server
#include "FGServer.h"				// FlightGear-server
#include "PPJoyServer.h"			// Virtual Joystick
#include "FTIRServer.h"				// FakeTIR-server
#include "SCServer.h"				// SimConnect-server (for MS Flight Simulator X)
#include "FSUIPCServer.h"			// FSUIPC-server (for MS Flight Simulator 2004)

// include the DirectX Library files
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

enum AngleName {
	PITCH = 0,
	YAW = 1,
	ROLL = 2,
	X = 3,
	Y = 4,
	Z = 5
};

enum FTNoIR_Client {
	FREE_TRACK = 0,
	FLIGHTGEAR = 1,
	FTNOIR = 2,
	PPJOY = 3,
	TRACKIR = 4,
	SIMCONNECT = 5,
	FSUIPC = 6
};

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

//
// Structure to hold all variables concerning one of 6 DOF's
//
struct THeadPoseDOF {
	float headPos;					// Current position (from faceTracker, radials or meters)
	float initial_headPos;			// Position on startup (first valid value)
	float offset_headPos;			// Offset for centering
	float invert;					// Invert measured value (= 1.0f or -1.0f)
	float red;						// Reduction factor (used for EWMA-filtering, between 0.0f and 1.0f)
	QList<float> rawList;			// List of 'n' headPos values (used for moving average)
	int maxItems;					// Maximum number of elements is rawList
	float prevPos;					// Previous Position
	float prevRawPos;				// Previous Raw Position
	QPainterPath curve;				// Bezier curve to translate input -> output
	int NeutralZone;				// Neutral zone
	int MaxInput;					// Maximum raw input
};

//
// Structure to hold keycode and CTRL, SHIFT, ALT for shortkeys
//
struct TShortKey {
	BYTE keycode;					// Required Key
	bool shift;						// Modifiers to examine
	bool ctrl;
	bool alt;
};

class Tracker : public QThread {
	Q_OBJECT

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	FTNoIR_Client selectedClient;

	/** face api variables **/
	APIScope *faceapi_scope;
    QSharedPointer<EngineBase> _engine;
	QSharedPointer<HeadTracker> _headtracker;
	smEngineHandle _engine_handle;

	/** static callback method for the head pose tracking **/
	static void STDCALL receiveHeadPose(void *,smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame);
	static void addRaw2List ( QList<float> *rawList, float maxIndex, float raw );
	static float lowPassFilter ( float newvalue, float *oldvalue, float dt, float coeff);
	static float rateLimiter ( float newvalue, float *oldvalue, float dt, float max_rate);
	static float getCorrectedNewRaw ( float NewRaw, float rotNeutral );

	/** static member variables for saving the head pose **/
	static THeadPoseDOF Pitch;						// Head-rotation X-direction (Up/Down)
	static THeadPoseDOF Yaw;						// Head-rotation Y-direction ()
	static THeadPoseDOF Roll;						// Head-rotation Z-direction ()
	static THeadPoseDOF X;							// Head-movement X-direction (Left/Right)
	static THeadPoseDOF Y;							// Head-movement Y-direction (Up/Down)
	static THeadPoseDOF Z;							// Head-movement Z-direction (To/From camera)

	static TShortKey CenterKey;						// ShortKey to Center headposition
	static TShortKey StartStopKey;					// ShortKey to Start/stop tracking

	// Flags to start/stop/reset tracking
	static bool confid;								// Tracker data is OK
	static bool set_initial;						// initial headpose is set
	static bool do_tracking;						// Start/stop tracking, using MINUS key on keyboard
	static bool do_center;							// Center head-position, using EQUALS key on keyboard

	static bool useFilter;
	static long prevHeadPoseTime;					// Time from previous sample
	
	/** QT objects **/
	QLineEdit *headXLine;
	QLineEdit *headYLine;
	QLineEdit *headZLine;

	QLineEdit *headRotXLine;
	QLineEdit *headRotYLine;
	QLineEdit *headRotZLine;

	QWidget *headPoseWidget;
	FaceTrackNoIR *mainApp;

	FTServer *server_FT;							// Freetrack Server
	FGServer *server_FG;							// FlightGear Server
	PPJoyServer *server_PPJoy;						// PPJoy Server
	FTIRServer *server_FTIR;						// Fake TIR Server
	SCServer *server_SC;							// SimConnect Server
	FSUIPCServer *server_FSUIPC;					// FSUIPC Server

protected:
	// qthread override run method 
	void run();

public:
	Tracker( int clientID );
	~Tracker();

	void setup(QWidget *head, FaceTrackNoIR *parent);
	void registerHeadPoseCallback();
	bool handleGameCommand ( int command );
	QString getGameProgramName();					// Get the ProgramName from the game and display it.
	void loadSettings();							// Load settings from the INI-file
	bool isShortKeyPressed( TShortKey *key, BYTE *keystate );

	QSharedPointer<EngineBase> getEngine() { return _engine; };

	static bool getConfid() { return confid; }

	static void setInvertPitch(bool invert) { Pitch.invert = invert?-1.0f:+1.0f; }
	static void setInvertYaw(bool invert) { Yaw.invert = invert?-1.0f:+1.0f; }
	static void setInvertRoll(bool invert) { Roll.invert = invert?-1.0f:+1.0f; }
	static void setInvertX(bool invert) { X.invert = invert?-1.0f:+1.0f; }
	static void setInvertY(bool invert) { Y.invert = invert?-1.0f:+1.0f; }
	static void setInvertZ(bool invert) { Z.invert = invert?-1.0f:+1.0f; }

	static void setUseFilter(bool set) { useFilter = set; }

	static void setRedYaw(int x) { Yaw.red = x/100.0f; }
	static void setRedPitch(int x) { Pitch.red = x/100.0f; }
	static void setRedRoll(int x) { Roll.red = x/100.0f; }
	static void setRedX(int x) { X.red = x/100.0f; }
	static void setRedY(int x) { Y.red = x/100.0f; }
	static void setRedZ(int x) { Z.red = x/100.0f; }

	static float getSmoothFromList ( QList<float> *rawList );
	static float getDegreesFromRads ( float rads ) { return (rads * 57.295781f); }
	static float getRadsFromDegrees ( float degrees ) { return (degrees * 0.017453f); }
	static float getOutputFromCurve ( QPainterPath *curve, float input, float neutralzone, float maxinput );

	// For now, use one slider for all
	void setSmoothing(int x) { 
		Pitch.maxItems = x;
		Yaw.maxItems = x;
		Roll.maxItems = x;
		X.maxItems = x;
		Y.maxItems = x;
		Z.maxItems = x;
	}

};

#endif