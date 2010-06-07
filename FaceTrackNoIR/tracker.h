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

#define DIRECTINPUT_VERSION 0x0800
#include <Dinput.h>

#include "FTServer.h"				// Freetrack-server
#include "FGServer.h"				// FlightGear-server

// include the DirectX Library files
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

//
// Structure to hold all variables concerning one of 6 DOF's
//
struct THeadPoseDOF {
	float headPos;					// Current position (from faceTracker, radials or meters)
	float initial_headPos;			// Position on startup (first valid value)
	float offset_headPos;			// Offset for centering
	float sens;						// Sensitivity (multiplication factor)
	float invert;					// Invert measured value (= 1.0f or -1.0f)
	float red;						// Reduction factor (used for EWMA-filtering, between 0.0f and 1.0f)
	QList<float> rawList;			// List of 'n' headPos values (used for moving average)
	int maxItems;					// Maximum number of elements is rawList
	float newPos;					// New Position (used locally)
	float prevPos;					// Previous Position
};

class Tracker : public QThread {
	Q_OBJECT

private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

	/** face api variables **/
	APIScope *faceapi_scope;
    QSharedPointer<EngineBase> _engine;
	QSharedPointer<HeadTracker> _headtracker;
	smEngineHandle _engine_handle;

	/** static callback method for the head pose tracking **/
	static void STDCALL receiveHeadPose(void *,smEngineHeadPoseData head_pose, smCameraVideoFrame video_frame);
	static void addRaw2List ( QList<float> *rawList, float maxIndex, float raw );
	static float lowPassFilter ( float newvalue, float *oldvalue, float dt, float coeff);
	static float getCorrectedNewRaw ( float NewRaw, float rotNeutral );

	/** static member variables for saving the head pose **/
	static THeadPoseDOF Pitch;						// Head-rotation X-direction (Up/Down)
	static THeadPoseDOF Yaw;						// Head-rotation Y-direction ()
	static THeadPoseDOF Roll;						// Head-rotation Z-direction ()
	static THeadPoseDOF X;							// Head-movement X-direction (Left/Right)
	static THeadPoseDOF Y;							// Head-movement Y-direction (Up/Down)
	static THeadPoseDOF Z;							// Head-movement Z-direction (To/From camera)

	// Flags to start/stop/reset tracking
	static bool confid;								// Tracker data is OK
	static bool set_initial;						// initial headpose is set
	static bool do_tracking;						// Start/stop tracking, using MINUS key on keyboard
	static bool do_center;							// Center head-position, using EQUALS key on keyboard

	static bool useFilter;
	static float rotNeutralZone;					// Neutral Zone for rotations (rad).
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

protected:
	// qthread override run method 
	void run();

public:
	Tracker();
	~Tracker();

	void setup(QWidget *head, FaceTrackNoIR *parent);
	void registerHeadPoseCallback();
	bool handleGameCommand ( int command );
	QString getGameProgramName();					// Get the ProgramName from the game and display it.

	QSharedPointer<EngineBase> getEngine() { return _engine; };
//	smEngineHandle getEngineHandle() { return _engine->handle(); };

	static float getHeadPosX() {return X.headPos;}
	static float getHeadPosY() {return Y.headPos;}
	static float getHeadPosZ() {return Z.headPos;}

	static void setHeadPosX(float x) { X.headPos = x; }
	static void setHeadPosY(float y) { Y.headPos = y; }
	static void setHeadPosZ(float z) { Z.headPos = z; }

	static float getHeadRotX() {return Pitch.headPos;}
	static float getHeadRotY() {return Yaw.headPos;}
	static float getHeadRotZ() {return Roll.headPos;}

	static void setHeadRotX(float x) { Pitch.headPos = x; }
	static void setHeadRotY(float y) { Yaw.headPos = y; }
	static void setHeadRotZ(float z) { Roll.headPos = z; }

	static bool getConfid() { return confid; }

	static void setSensPitch(int x) { Pitch.sens = x/100.0f; }
	static void setSensYaw(int x) { Yaw.sens = x/100.0f; }
	static void setSensRoll(int x) { Roll.sens = x/100.0f; }
	static void setSensX(int x) { X.sens = x/100.0f; }
	static void setSensY(int x) { Y.sens = x/100.0f; }
	static void setSensZ(int x) { Z.sens = x/100.0f; }

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

	static void setNeutralZone(int x) { rotNeutralZone = (x * 2.0f * 3.14159)/360.0f; }

	float getSmoothFromList ( QList<float> *rawList );
	float getDegreesFromRads ( float rads ) { return ((rads * 360.0f)/ (2.0f * 3.14159)); }

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