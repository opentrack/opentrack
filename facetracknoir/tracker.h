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

#include <qfunctionconfigurator/functionconfig.h>
#include "tracker_types.h"

class FaceTrackNoIR;				// pre-define parent-class to avoid circular includes

//
// Structure to hold all variables concerning one of 6 DOF's
//
class THeadPoseDOF {
private:
    THeadPoseDOF(const THeadPoseDOF &) {}
public:
    THeadPoseDOF() :
        headPos(0),
        invert(0),
        altp(false),
        zero(0)
    {
    }

    THeadPoseDOF(QString primary,
                 QString secondary,
                 int maxInput1,
                 int maxOutput1,
                 int maxInput2,
                 int maxOutput2) :
        headPos(0),
        invert(1),
        curve(primary, maxInput1, maxOutput1),
        curveAlt(secondary, maxInput2, maxOutput2),
        zero(0)
    {
        QSettings settings("opentrack");							// Registry settings (in HK_USER)
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );
        curve.loadSettings(iniFile);
        curveAlt.loadSettings(iniFile);
        
        iniFile.beginGroup("Tracking");
        altp = iniFile.value(secondary).toBool();
        iniFile.endGroup();
    }
    volatile double headPos;					// Current position (from faceTracker, radials or meters)
    volatile float invert;					// Invert measured value (= 1.0f or -1.0f)
	FunctionConfig curve;		// Function to translate input -> output
	FunctionConfig curveAlt;
    volatile bool altp;
    volatile double zero;
};

class Tracker : public QThread {
	Q_OBJECT

private:
    FaceTrackNoIR *mainApp;
    QMutex mtx;

protected:
	// qthread override run method 
	void run();

public:
	Tracker( FaceTrackNoIR *parent );
    ~Tracker();

    void setInvertAxis(Axis axis, bool invert);

    void getHeadPose(double *data);				// Return the current headpose data
    void getOutputHeadPose(double *data);			// Return the current (processed) headpose data

    volatile bool should_quit;
    // following are now protected by hTrackMutex
    volatile bool do_center;							// Center head-position, using the shortkey
    volatile bool enabled;
    
    T6DOF output_camera;
};

class HeadPoseData {
public:
    THeadPoseDOF* axes[6];
    HeadPoseData()
    {
        axes[TX] = new THeadPoseDOF("tx","tx_alt", 100, 100, 100, 100);
        axes[TY] = new THeadPoseDOF("ty","ty_alt", 100, 100, 100, 100);
        axes[TZ] = new THeadPoseDOF("tz","tz_alt", 100, 100, 100, 100);
        axes[Yaw] = new THeadPoseDOF("rx", "rx_alt", 180, 180, 180, 180);
        axes[Pitch] = new THeadPoseDOF("ry", "ry_alt", 90, 90, 90, 90);
        axes[Roll] = new THeadPoseDOF("rz", "rz_alt", 180, 180, 180, 180);
    }
};

#endif
