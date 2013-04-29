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
#include <QPoint>
#include <QWaitCondition>
#include <QList>
#include <QPainterPath>
#include <QDebug>
#include <QMutex>
#include "global-settings.h"
#include <ftnoir_tracker_base/ftnoir_tracker_types.h>

//#define DIRECTINPUT_VERSION 0x0800
//#include <Dinput.h>
#undef FTNOIR_PROTOCOL_BASE_LIB
#undef FTNOIR_TRACKER_BASE_LIB
#undef FTNOIR_FILTER_BASE_LIB
#undef FTNOIR_PROTOCOL_BASE_EXPORT
#undef FTNOIR_TRACKER_BASE_EXPORT
#undef FTNOIR_FILTER_BASE_EXPORT
#define FTNOIR_PROTOCOL_BASE_EXPORT Q_DECL_IMPORT
#define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_IMPORT
#define FTNOIR_FILTER_BASE_EXPORT Q_DECL_IMPORT

#include <qfunctionconfigurator/functionconfig.h>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_protocol_base/ftnoir_protocol_base.h"
#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "tracker_types.h"

// include the DirectX Library files
//#pragma comment (lib, "dinput8.lib")
//#pragma comment (lib, "dxguid.lib")

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

struct HeadPoseData;
extern HeadPoseData* GlobalPose;

//
// Structure to hold all variables concerning one of 6 DOF's
//
class THeadPoseDOF {
public:
    THeadPoseDOF(QString primary = "", QString secondary= "", int maxInput1 = 50, int maxOutput1 = 180, int maxInput2 = 50, int maxOutput2 = 90) {
        QSettings settings("opentrack");							// Registry settings (in HK_USER)
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );								// Application settings (in INI-file)

        if (primary != "")
        {
            curvePtr = new FunctionConfig(primary, maxInput1, maxOutput1);						// Create the Function-config for input-output translation
            curvePtr->loadSettings(iniFile);													// Load the settings from the INI-file
        }

        if (secondary != "")
        {
            curvePtrAlt = new FunctionConfig(secondary, maxInput2, maxOutput2);
            curvePtrAlt->loadSettings(iniFile);
            iniFile.beginGroup("Tracking");
            altp = iniFile.value(secondary).toBool();
            iniFile.endGroup();
        }
        headPos = 0.0f;
        invert = 1;
    }
	float headPos;					// Current position (from faceTracker, radials or meters)
    float invert;					// Invert measured value (= 1.0f or -1.0f)
	FunctionConfig* curvePtr;		// Function to translate input -> output
	FunctionConfig* curvePtrAlt;
    bool altp;
};

class Tracker : public QThread {
	Q_OBJECT

private:
    FaceTrackNoIR *mainApp;

protected:
	// qthread override run method 
	void run();

public:
	Tracker( FaceTrackNoIR *parent );
    ~Tracker();
	void loadSettings();							// Load settings from the INI-file
    bool getTrackingActive() { return confid; }

    void setInvertAxis(Axis axis, bool invert);

    void getHeadPose(double *data);				// Return the current headpose data
    void getOutputHeadPose(double *data);			// Return the current (processed) headpose data

    float getDegreesFromRads ( float rads ) { return (rads * 57.295781f); }
    float getRadsFromDegrees ( float degrees ) { return (degrees * 0.017453f); }
    volatile bool should_quit;
    // following are now protected by hTrackMutex
    volatile bool do_center;							// Center head-position, using the shortkey
    // Flags to start/stop/reset tracking
    volatile bool confid;                                // Tracker data is OK;
    
    T6DOF output_camera;
};

struct HeadPoseData {
public:
    THeadPoseDOF axes[6];
    HeadPoseData()
    {
        axes[TX] = THeadPoseDOF("tx","tx_alt", 100, 100, 100, 100);
        axes[TY] = THeadPoseDOF("ty","ty_alt", 100, 100, 100, 100);
        axes[TZ] = THeadPoseDOF("tz","tz_alt", 100, 100, 100, 100);
        axes[RX] = THeadPoseDOF("rx", "rx_alt", 180, 180, 180, 180);
        axes[RY] = THeadPoseDOF("ry", "ry_alt", 180, 180, 180, 180);
        axes[RZ] = THeadPoseDOF("rz", "rz_alt", 180, 180, 180, 180);
    }
};

#endif
