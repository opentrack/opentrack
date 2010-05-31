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
//#pragma comment (lib, "d3d9.lib")
//#pragma comment (lib, "d3dx9.lib")
#pragma comment (lib, "dinput8.lib")
#pragma comment (lib, "dxguid.lib")

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

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

	/** static member varables for saving the head pose **/
	static float headPosX;
	static float headPosY;
	static float headPosZ;							// Distance from camera
	static float initial_headPosZ;					// Initial distance when headpose is valid

	static float headRotX;
	static float headRotY;
	static float headRotZ;
	static bool confid;
	static bool set_initial;						// initial headpose is set

	/** static member varables for calculating the virtual head pose **/
	static float sensYaw;
	static float sensPitch;
	static float sensRoll;
	static float sensX;
	static float sensY;
	static float sensZ;

	static float invertYaw;
	static float invertPitch;
	static float invertRoll;
	static float invertX;
	static float invertY;
	static float invertZ;

	static float rotNeutralZone;						// Neutral Zone for rotations (rad).
	
	//
	// The Raw FaceAPI-data may need smoothing.
	// We implement smoothing, by taking the last 'x' raw samples and making an average of this.
	// After 'x' samples, the oldest sample is removed and the fresh 'pre-pended' to the QList
	//
	QList<float> rawYawList;							// List of last 'x' values from FaceAPI
	QList<float> rawPitchList;
	QList<float> rawRollList;
	QList<float> rawXList;
	QList<float> rawYList;
	QList<float> rawZList;

	int intMaxYawItems;									// Max. number of items in QList: more = smoother (yet slower!)
	int intMaxPitchItems;
	int intMaxRollItems;
	int intMaxXItems;
	int intMaxYItems;
	int intMaxZItems;

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

	static float getHeadPosX() {return headPosX;}
	static float getHeadPosY() {return headPosY;}
	static float getHeadPosZ() {return headPosZ;}

	static void setHeadPosX(float x) { headPosX = x; }
	static void setHeadPosY(float y) { headPosY = y; }
	static void setHeadPosZ(float z) { headPosZ = z; }

	static float getHeadRotX() {return headRotX;}
	static float getHeadRotY() {return headRotY;}
	static float getHeadRotZ() {return headRotZ;}

	static void setHeadRotX(float x) { headRotX = x; }
	static void setHeadRotY(float y) { headRotY = y; }
	static void setHeadRotZ(float z) { headRotZ = z; }

	static bool getConfid() { return confid; }

	static void setSensYaw(int x) { sensYaw = x/100.0f; }
	static void setSensPitch(int x) { sensPitch = x/100.0f; }
	static void setSensRoll(int x) { sensRoll = x/100.0f; }
	static void setSensX(int x) { sensX = x/100.0f; }
	static void setSensY(int x) { sensY = x/100.0f; }
	static void setSensZ(int x) { sensZ = x/100.0f; }

	static void setInvertYaw(bool invert) { invertYaw = invert?-1.0f:+1.0f; }
	static void setInvertPitch(bool invert) { invertPitch = invert?-1.0f:+1.0f; }
	static void setInvertRoll(bool invert) { invertRoll = invert?-1.0f:+1.0f; }
	static void setInvertX(bool invert) { invertX = invert?-1.0f:+1.0f; }
	static void setInvertY(bool invert) { invertY = invert?-1.0f:+1.0f; }
	static void setInvertZ(bool invert) { invertZ = invert?-1.0f:+1.0f; }

	static void setNeutralZone(int x) { rotNeutralZone = (x * 2.0f * 3.14159)/360.0f; }

	void addRaw2List ( QList<float> *rawList, float maxIndex, float raw );
	float getSmoothFromList ( QList<float> *rawList );
	float getCorrectedNewRaw ( float NewRaw, float rotNeutral );
	float getDegreesFromRads ( float rads ) { return ((rads * 360.0f)/ (2.0f * 3.14159)); }

	// For now, use one slider for all
	void setSmoothing(int x) { 
		intMaxYawItems = x;
		intMaxPitchItems = x;
		intMaxRollItems = x;
		intMaxXItems = x;
		intMaxYItems = x;
		intMaxZItems = x;
	}

};

#endif