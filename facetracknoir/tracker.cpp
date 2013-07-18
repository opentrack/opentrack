/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
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
		20121215 - WVR: Fixed crash after message: protocol not installed correctly... by terminating the thread.
		20120921 - WVR: Fixed centering when no filter is selected.
		20120917 - WVR: Added Mouse-buttons to ShortKeys.
		20120827 - WVR: Signal tracking = false to Curve-widget(s) when quitting run(). Also when Alternative Pitch curve is used.
		20120805 - WVR: The FunctionConfig-widget is used to configure the Curves. It was tweaked some more, because the Accela filter now also
						uses the Curve(s). ToDo: make the ranges configurable by the user. Development on the Toradex IMU makes us realize, that
						a fixed input-range may not be so handy after all..
		20120427 - WVR: The Protocol-code was already in separate DLLs, but the ListBox was still filled �statically�. Now, a Dir() of the
						EXE-folder is done, to locate Protocol-DLLs. The Icons were also moved to the DLLs
		20120317 - WVR: The Filter and Tracker-code was moved to separate DLLs. The calling-method
						was changed accordingly.
						The face-tracker member-functions NotifyZeroed and refreshVideo were added, as 
						requested by Stanislaw.
		20110411 - WVR: Finished moving all Protocols to separate C++ projects. Every protocol now
						has it's own Class, that's inside it's own DLL. This reduces the size of the program,
						makes it more structured and enables a more sophisticated installer.
		20110328 - WVR: Changed the camera-structs into class-instances. This makes initialisation
						easier and hopefully solves the remaining 'start-up problem'.
		20110313 - WVR: Removed 'set_initial'. Less is more.
		20110109 - WVR: Added setZero option to define behaviour after STOP tracking via shortkey.
		20110104 - WVR: Removed a few nasty bugs (it was impossible to stop tracker without crash).
		20101224 - WVR: Removed the QThread inheritance of the Base Class for the protocol-servers.
						Again, this drastically simplifies the code in the protocols.
		20101217 - WVR: Created Base Class for the protocol-servers. This drastically simplifies
						the code needed here.
		20101024 - WVR: Added shortkey to disable/enable one or more axis during tracking.
		20101021 - WVR: Added FSUIPC server for FS2004.
		20101011 - WVR: Added SimConnect server.
		20101007 - WVR: Created 6DOF-curves and drastically changed the tracker for that.
						Also eliminated a 'glitch' in the process.
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
#include "facetracknoir.h"

/** constructor **/
Tracker::Tracker( FaceTrackNoIR *parent ) :
    confid(false),
    should_quit(false),
    do_center(false)
{
    // Retieve the pointer to the parent
	mainApp = parent;
	// Load the settings from the INI-file
	loadSettings();
}

Tracker::~Tracker()
{
}

static void get_curve(double pos, double& out, THeadPoseDOF& axis) {
    bool altp = (pos < 0) && axis.altp;
    if (altp) {
        out = axis.invert * axis.curveAlt.getValue(pos);
        axis.curve.setTrackingActive( false );
        axis.curveAlt.setTrackingActive( true );
    }
    else {
        out = axis.invert * axis.curve.getValue(pos);
        axis.curve.setTrackingActive( true );
        axis.curveAlt.setTrackingActive( false );
    }
    out += axis.zero;
}

/** QThread run method @override **/
void Tracker::run() {
    T6DOF current_camera;                   // Used for filtering
    T6DOF target_camera;
    T6DOF new_camera;
    
    /** Direct Input variables **/
    T6DOF offset_camera;
    T6DOF gameoutput_camera;
    
    bool bTracker1Confid = false;
    bool bTracker2Confid = false;
    
    double newpose[6];
    double last_post_filter[6];
    
    forever
	{
        if (should_quit)
            break;

        for (int i = 0; i < 6; i++)
            newpose[i] = 0;

        //
        // The second tracker serves as 'secondary'. So if an axis is written by the second tracker it CAN be overwritten by the Primary tracker.
        // This is enforced by the sequence below.
        //
        if (Libraries->pSecondTracker) {
            bTracker2Confid = Libraries->pSecondTracker->GiveHeadPoseData(newpose);
        }

        if (Libraries->pTracker) {
            bTracker1Confid = Libraries->pTracker->GiveHeadPoseData(newpose);
        }

        confid = bTracker1Confid || bTracker2Confid;

        if ( confid ) {
            for (int i = 0; i < 6; i++)
                mainApp->axis(i).headPos = newpose[i];
        }

        //
        // If Center is pressed, copy the current values to the offsets.
        //
        if (do_center)  {
            //
            // Only copy valid values
            //
            if (confid) {
                for (int i = 0; i < 6; i++)
                    offset_camera.axes[i] = mainApp->axis(i).headPos;
            }

            Tracker::do_center = false;
            
            if (Libraries->pTracker)
                Libraries->pTracker->NotifyCenter();
            
            if (Libraries->pSecondTracker)
                Libraries->pSecondTracker->NotifyCenter();
            
            if (Libraries->pFilter)
                Libraries->pFilter->Initialize();
        }

        if (getTrackingActive()) {
            QMutexLocker foo(&mtx);
            
            // get values
            for (int i = 0; i < 6; i++)
                target_camera.axes[i] = mainApp->axis(i).headPos;

            // do the centering
            target_camera = target_camera - offset_camera;

            //
            // Use advanced filtering, when a filter was selected.
            //
            if (Libraries->pFilter) {
                for (int i = 0; i < 6; i++)
                    last_post_filter[i] = gameoutput_camera.axes[i];
                Libraries->pFilter->FilterHeadPoseData(current_camera.axes, target_camera.axes, new_camera.axes, last_post_filter);
            }
            else {
                new_camera = target_camera;
            }

            for (int i = 0; i < 6; i++) {
                get_curve(new_camera.axes[i], output_camera.axes[i], mainApp->axis(i));
            }

            //
            // Send the headpose to the game
            //
            if (Libraries->pProtocol) {
                gameoutput_camera = output_camera;
                Libraries->pProtocol->sendHeadposeToGame( gameoutput_camera.axes, newpose );	// degrees & centimeters
            }
        }

        //for lower cpu load
        msleep(1);
    }

    for (int i = 0; i < 6; i++)
    {
        mainApp->axis(i).curve.setTrackingActive(false);
        mainApp->axis(i).curveAlt.setTrackingActive(false);
    }
}

//
// Get the raw headpose, so it can be displayed.
//
void Tracker::getHeadPose( double *data ) {
    QMutexLocker foo(&mtx);
    for (int i = 0; i < 6; i++)
    {
        data[i] = mainApp->axis(i).headPos;
    }
}

//
// Get the output-headpose, so it can be displayed.
//
void Tracker::getOutputHeadPose( double *data ) {
    QMutexLocker foo(&mtx);
    for (int i = 0; i < 6; i++)
        data[i] = output_camera.axes[i];
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void Tracker::loadSettings() {
	qDebug() << "Tracker::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    iniFile.beginGroup("Tracking");
    
	qDebug() << "loadSettings says: iniFile = " << currentFile;
    
    const char* names2[] = {
        "zero_tx",
        "zero_ty",
        "zero_tz",
        "zero_rx",
        "zero_ry",
        "zero_rz"
    };
    
    for (int i = 0; i < 6; i++)
        mainApp->axis(i).zero = iniFile.value(names2[i], 0).toDouble();
    
    iniFile.endGroup();
}

void Tracker::setInvertAxis(Axis axis, bool invert) { mainApp->axis(axis).invert = invert?-1.0f:1.0f; }
