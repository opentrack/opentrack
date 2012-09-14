/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010 - 2012	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
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
/*
	Modifications (last one on top):
		20120717 - WVR: FunctionConfig is now used for the Curves, instead of BezierConfig.
*/
#ifndef __TRACKER_H__
#define __TRACKER_H__

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

#include "ExcelServer.h"			// Excel-server (for analysing purposes)
#include "FTNoIR_cxx_protocolserver.h"
#include "FunctionConfig.h"

#include "..\ftnoir_tracker_base\FTNoIR_Tracker_base.h"
#include "..\ftnoir_protocol_base\FTNoIR_Protocol_base.h"
#include "..\ftnoir_filter_base\FTNoIR_Filter_base.h"
//#include "AutoClosePtr.h"

// 1a. COM-Like usage with smart pointer.
// No need to call `ITracker::Release'; the instance will
// be released automatically in destructor of the smart pointer.
//typedef AutoClosePtr<ITracker, void, &ITracker::Release> ITrackerPtr;
typedef ITracker *(WINAPI *importGetTracker)(void);
typedef AutoClosePtr<IProtocol, void, &IProtocol::Release> IProtocolPtr;
typedef IProtocol *(WINAPI *importGetProtocol)(void);


// include the DirectX Library files
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

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
	FSUIPC = 6,
	MOUSE = 7
};

//enum FTNoIR_Face_Tracker {
//	FT_SM_FACEAPI = 0,
//	FT_FTNOIR = 1,
//	FT_VISAGE = 2
//};

enum FTNoIR_Tracker_Status {
	TRACKER_OFF = 0,
	TRACKER_ON = 1
};

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

//
// Structure to hold all variables concerning one of 6 DOF's
//
class THeadPoseDOF {
public:

	THeadPoseDOF(QString primary, QString secondary = "", int maxInput = 50, int maxOutput = 180) {
		QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");							// Registry settings (in HK_USER)
		QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
		QSettings iniFile( currentFile, QSettings::IniFormat );								// Application settings (in INI-file)

		curvePtr = new FunctionConfig(primary, maxInput, maxOutput);						// Create the Function-config for input-output translation
		curvePtr->loadSettings(iniFile);													// Load the settings from the INI-file
		if (secondary != "") {
			curvePtrAlt = new FunctionConfig(secondary, maxInput, maxOutput);
			curvePtrAlt->loadSettings(iniFile);
		}

	}

	void initHeadPoseData(){
		headPos = 0.0f;
		offset_headPos = 0.0f;
		invert = 0.0f;
		red = 0.0f;
		rawList.clear();
		maxItems = 10.0f;
		prevPos = 0.0f;
		prevRawPos = 0.0f;
		NeutralZone = 0;
		MaxInput = 0;
		confidence = 0.0f;
		newSample = FALSE;

		qDebug() << "initHeadPoseData: " << curvePtr->getTitle();

	}
	float headPos;					// Current position (from faceTracker, radials or meters)
	float offset_headPos;			// Offset for centering
	float invert;					// Invert measured value (= 1.0f or -1.0f)
	float red;						// Reduction factor (used for EWMA-filtering, between 0.0f and 1.0f)
	QList<float> rawList;			// List of 'n' headPos values (used for moving average)
	int maxItems;					// Maximum number of elements in rawList
	float prevPos;					// Previous Position
	float prevRawPos;				// Previous Raw Position
//	QPainterPath curve;				// Bezier curve to translate input -> output

	FunctionConfig* curvePtr;		// Function to translate input -> output
	FunctionConfig* curvePtrAlt;

	int NeutralZone;				// Neutral zone
	int MaxInput;					// Maximum raw input
	float confidence;				// Current confidence
	bool newSample;					// Indicate new sample from tracker
};

//
// Structure to hold keycode and CTRL, SHIFT, ALT for shortkeys
//
struct TShortKey {
	BYTE keycode;					// Required Key
	bool shift;						// Modifiers to examine
	bool ctrl;
	bool alt;
	bool doPitch;					// Modifiers to act on axis
	bool doYaw;
	bool doRoll;
	bool doX;
	bool doY;
	bool doZ;
};

class Tracker : public QThread {
	Q_OBJECT

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	static T6DOF current_camera;					// Used for filtering
	static T6DOF target_camera;
	static T6DOF new_camera;
	static T6DOF output_camera;

	ITracker *pTracker;								// Pointer to Tracker instance (in DLL)
	static IProtocolPtr pProtocol;					// Pointer to Protocol instance (in DLL)
	static IFilterPtr pFilter;						// Pointer to Filter instance (in DLL)

	static void addHeadPose( THeadPoseData head_pose );
	static void addRaw2List ( QList<float> *rawList, float maxIndex, float raw );

	static TShortKey CenterKey;						// ShortKey to Center headposition
	static TShortKey StartStopKey;					// ShortKey to Start/stop tracking
	static TShortKey InhibitKey;					// ShortKey to disable one or more axis during tracking
	static TShortKey GameZeroKey;					// ShortKey to Set Game Zero
//	static TShortKey AxisReverseKey;				// ShortKey to reverse axis during tracking

	// Flags to start/stop/reset tracking
	static bool confid;								// Tracker data is OK
	static bool do_tracking;						// Start/stop tracking, using the shortkey
	static bool do_center;							// Center head-position, using the shortkey
	static bool do_inhibit;							// Inhibit DOF-axis, using the shortkey
	static bool do_game_zero;						// Set in-game zero, using the shortkey
	static bool do_axis_reverse;					// Axis reverse, using the shortkey

	static HANDLE hTrackMutex;						// Prevent reading/writing the headpose simultaneously

	static bool useFilter;							// Use EWMA-filtering
	static bool setZero;							// Set to zero's, when OFF (one-shot)
	static bool setEngineStop;						// Stop tracker->engine, when OFF

	static bool useAxisReverse;						// Use Axis Reverse
	static float YawAngle4ReverseAxis;				// Axis Reverse settings
	static float Z_Pos4ReverseAxis;
	static float Z_PosWhenReverseAxis;

	FaceTrackNoIR *mainApp;

	QSharedPointer<ProtocolServerBase> debug_Client;	// Protocol Server to log debug-data

protected:
	// qthread override run method 
	void run();

public:
	Tracker( FaceTrackNoIR *parent );
	~Tracker();

	/** static member variables for saving the head pose **/
	static THeadPoseDOF Pitch;						// Head-rotation X-direction (Up/Down)
	static THeadPoseDOF Yaw;						// Head-rotation Y-direction ()
	static THeadPoseDOF Roll;						// Head-rotation Z-direction ()
	static THeadPoseDOF X;							// Head-movement X-direction (Left/Right)
	static THeadPoseDOF Y;							// Head-movement Y-direction (Up/Down)
	static THeadPoseDOF Z;							// Head-movement Z-direction (To/From camera)

	void setup();

//	void registerHeadPoseCallback();
	bool handleGameCommand ( int command );
	QString getGameProgramName();					// Get the ProgramName from the game and display it.
	void loadSettings();							// Load settings from the INI-file
	bool isShortKeyPressed( TShortKey *key, BYTE *keystate );

	static bool getTrackingActive() { return do_tracking && confid; }
	static bool getAxisReverse() { return do_axis_reverse; }

	static bool getConfid() { return confid; }

	static void setInvertPitch(bool invert) { Pitch.invert = invert?-1.0f:+1.0f; }
	static void setInvertYaw(bool invert) { Yaw.invert = invert?-1.0f:+1.0f; }
	static void setInvertRoll(bool invert) { Roll.invert = invert?-1.0f:+1.0f; }
	static void setInvertX(bool invert) { X.invert = invert?-1.0f:+1.0f; }
	static void setInvertY(bool invert) { Y.invert = invert?-1.0f:+1.0f; }
	static void setInvertZ(bool invert) { Z.invert = invert?-1.0f:+1.0f; }

	static void setUseFilter(bool set) { useFilter = set; }
	static void getHeadPose(THeadPoseData *data);				// Return the current headpose data
	static void getOutputHeadPose(THeadPoseData *data);			// Return the current (processed) headpose data
	static IFilterPtr getFilterPtr() { return pFilter; }		// Return the pointer for the active Filter
	ITracker *getTrackerPtr() { return pTracker; }				// Return the pointer for the active Tracker

	void doRefreshVideo() {										// Call the face-tracker-function RefreshVideo
		if (pTracker) {
			pTracker->refreshVideo();
		}
	};

	static float getSmoothFromList ( QList<float> *rawList );
	static float getDegreesFromRads ( float rads ) { return (rads * 57.295781f); }
	static float getRadsFromDegrees ( float degrees ) { return (degrees * 0.017453f); }
//	static float getOutputFromCurve ( QPainterPath *curve, float input, float neutralzone, float maxinput );

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