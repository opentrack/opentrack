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

HeadPoseData* GlobalPose = NULL;

/** constructor **/
Tracker::Tracker( FaceTrackNoIR *parent ) :
    confid(false),
    useAxisReverse(false),
    YawAngle4ReverseAxis(40),
    Z_Pos4ReverseAxis(-20.0f),
    Z_PosWhenReverseAxis(50.0),
    should_quit(false),
    do_tracking(true),
    do_center(false),
    do_inhibit(false),
    do_game_zero(false),
    do_axis_reverse(false),
    inhibit_rx(false),
    inhibit_ry(false),
    inhibit_rz(false),
    inhibit_tx(false),
    inhibit_ty(false),
    inhibit_tz(false)
{
    // Retieve the pointer to the parent
	mainApp = parent;
	// Load the settings from the INI-file
	loadSettings();
    GlobalPose->Yaw.headPos = 0;
    GlobalPose->Pitch.headPos = 0;
    GlobalPose->Roll.headPos = 0;
    GlobalPose->X.headPos = 0;
    GlobalPose->Y.headPos = 0;
    GlobalPose->Z.headPos = 0;
}

Tracker::~Tracker()
{
}

static void get_curve(bool inhibitp, bool inhibit_zerop, double pos, double& out, THeadPoseDOF& axis) {
    if (inhibitp) {
        if (inhibit_zerop)
            out = 0;
        axis.curvePtr->setTrackingActive( true );
        axis.curvePtrAlt->setTrackingActive( false );
    }
    else {
        bool altp = (pos < 0) && axis.altp;
        if (altp) {
            out = axis.invert * axis.curvePtrAlt->getValue(pos);
            axis.curvePtr->setTrackingActive( false );
            axis.curvePtrAlt->setTrackingActive( true );
        }
        else {
            out = axis.invert * axis.curvePtr->getValue(pos);
            axis.curvePtr->setTrackingActive( true );
            axis.curvePtrAlt->setTrackingActive( false );
        }
    }
}

/** QThread run method @override **/
void Tracker::run() {
    T6DOF current_camera;                   // Used for filtering
    T6DOF target_camera;
    T6DOF new_camera;
    
    /** Direct Input variables **/
    T6DOF offset_camera;
    T6DOF gamezero_camera;
    T6DOF gameoutput_camera;
    
    bool bTracker1Confid = false;
    bool bTracker2Confid = false;
    
    THeadPoseData last;
    THeadPoseData newpose;
    THeadPoseData last_post_filter;
    
    forever
	{
        if (should_quit)
            break;

        newpose.pitch = 0.0f;
        newpose.roll = 0.0f;
        newpose.yaw = 0.0f;
        newpose.x = 0.0f;
        newpose.y = 0.0f;
        newpose.z = 0.0f;

        //
        // The second tracker serves as 'secondary'. So if an axis is written by the second tracker it CAN be overwritten by the Primary tracker.
        // This is enforced by the sequence below.
        //
        if (Libraries->pSecondTracker) {
            bTracker2Confid = Libraries->pSecondTracker->GiveHeadPoseData(&newpose);
        }

        if (Libraries->pTracker) {
            bTracker1Confid = Libraries->pTracker->GiveHeadPoseData(&newpose);
        }
        
        confid = (bTracker1Confid || bTracker2Confid);
        
        bool newp = last.yaw != newpose.yaw ||
                               last.pitch != newpose.pitch ||
                               last.roll != newpose.roll ||
                               last.x != newpose.x ||
                               last.y != newpose.y ||
                               last.z != newpose.z;
        
        if (newp)
            last = newpose;
                    
        if ( confid ) {
            GlobalPose->Yaw.headPos = newpose.yaw;
            GlobalPose->Pitch.headPos = newpose.pitch;
            GlobalPose->Roll.headPos = newpose.roll;
            GlobalPose->X.headPos = newpose.x;
            GlobalPose->Y.headPos = newpose.y;
            GlobalPose->Z.headPos = newpose.z;
        }

        //
        // If Center is pressed, copy the current values to the offsets.
        //
        if (do_center)  {
            //
            // Only copy valid values
            //
            if (confid) {
                offset_camera.x     = GlobalPose->X.headPos;
                offset_camera.y     = GlobalPose->Y.headPos;
                offset_camera.z     = GlobalPose->Z.headPos;
                offset_camera.pitch = GlobalPose->Pitch.headPos;
                offset_camera.yaw   = GlobalPose->Yaw.headPos;
                offset_camera.roll  = GlobalPose->Roll.headPos;
            }

            Tracker::do_center = false;
            
            // for kalman
            if (Libraries->pFilter)
                Libraries->pFilter->Initialize();
            
            last = newpose;
        }
        
        if (do_game_zero) {
            gamezero_camera = gameoutput_camera;
            do_game_zero = false;
        }

        if (do_tracking && confid) {
            // get values
            target_camera.x     = GlobalPose->X.headPos;
            target_camera.y     = GlobalPose->Y.headPos;
            target_camera.z     = GlobalPose->Z.headPos;
            target_camera.pitch = GlobalPose->Pitch.headPos;
            target_camera.yaw   = GlobalPose->Yaw.headPos;
            target_camera.roll  = GlobalPose->Roll.headPos;

            // do the centering
            target_camera = target_camera - offset_camera;

            //
            // Use advanced filtering, when a filter was selected.
            //
            if (Libraries->pFilter) {
                last_post_filter = gameoutput_camera;
                Libraries->pFilter->FilterHeadPoseData(&current_camera, &target_camera, &new_camera, &last_post_filter, newp);
            }
            else {
                new_camera = target_camera;
            }

            get_curve(do_inhibit && inhibit_rx, inhibit_zero, new_camera.yaw, output_camera.yaw, GlobalPose->Yaw);
            get_curve(do_inhibit && inhibit_ry, inhibit_zero, new_camera.pitch, output_camera.pitch, GlobalPose->Pitch);
            get_curve(do_inhibit && inhibit_rz, inhibit_zero, new_camera.roll, output_camera.roll, GlobalPose->Roll);
            get_curve(do_inhibit && inhibit_tx, inhibit_zero, new_camera.x, output_camera.x, GlobalPose->X);
            get_curve(do_inhibit && inhibit_ty, inhibit_zero, new_camera.y, output_camera.y, GlobalPose->Y);
            get_curve(do_inhibit && inhibit_tz, inhibit_zero, new_camera.z, output_camera.z, GlobalPose->Z);

            if (useAxisReverse) {
                do_axis_reverse = ((fabs(output_camera.yaw) > YawAngle4ReverseAxis) && (output_camera.z < Z_Pos4ReverseAxis));
            } else {
                do_axis_reverse = false;
            }

            //
            // Reverse Axis.
            //
            if (do_axis_reverse) {
                output_camera.z = Z_PosWhenReverseAxis;	// Set the desired Z-position
            }

            //
            // Send the headpose to the game
            //
            if (Libraries->pProtocol) {
                gameoutput_camera = output_camera + gamezero_camera;
                Libraries->pProtocol->sendHeadposeToGame( &gameoutput_camera, &newpose );	// degrees & centimeters
            }
        }
        else {
            //
            // Go to initial position
            //
            if (Libraries->pProtocol && inhibit_zero) {
                output_camera.pitch = 0.0f;
                output_camera.yaw = 0.0f;
                output_camera.roll = 0.0f;
                output_camera.x = 0.0f;
                output_camera.y = 0.0f;
                output_camera.z = 0.0f;
                gameoutput_camera = output_camera + gamezero_camera;
                Libraries->pProtocol->sendHeadposeToGame( &gameoutput_camera, &newpose );				// degrees & centimeters
            }
            GlobalPose->X.curvePtr->setTrackingActive( false );
            GlobalPose->Y.curvePtr->setTrackingActive( false );
            GlobalPose->Z.curvePtr->setTrackingActive( false );
            GlobalPose->Yaw.curvePtr->setTrackingActive( false );
            GlobalPose->Pitch.curvePtr->setTrackingActive( false );
            GlobalPose->Pitch.curvePtrAlt->setTrackingActive( false );
            GlobalPose->Roll.curvePtr->setTrackingActive( false );
            if (Libraries->pFilter)
                Libraries->pFilter->Initialize();
        }

        //for lower cpu load
        usleep(1000);
    }

    GlobalPose->X.curvePtr->setTrackingActive( false );
    GlobalPose->Y.curvePtr->setTrackingActive( false );
    GlobalPose->Z.curvePtr->setTrackingActive( false );
    GlobalPose->Yaw.curvePtr->setTrackingActive( false );
    GlobalPose->Pitch.curvePtr->setTrackingActive( false );
    GlobalPose->Pitch.curvePtrAlt->setTrackingActive( false );
    GlobalPose->Roll.curvePtr->setTrackingActive( false );
}

//
// Get the ProgramName from the Game and return it.
//
QString Tracker::getGameProgramName() {
QString str;
char dest[100];

	str = QString("No protocol active?");
    if (Libraries->pProtocol) {
        Libraries->pProtocol->getNameFromGame( dest );
		str = QString( dest );
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
			Tracker::do_center = true;
			break;
		default:
			break;
	}
	return false;
}

//
// Get the raw headpose, so it can be displayed.
//
void Tracker::getHeadPose( THeadPoseData *data ) {
    data->x = GlobalPose->X.headPos;				// centimeters
    data->y = GlobalPose->Y.headPos;
    data->z = GlobalPose->Z.headPos;

    data->pitch = GlobalPose->Pitch.headPos;		// degrees
    data->yaw = GlobalPose->Yaw.headPos;
    data->roll = GlobalPose->Roll.headPos;
}

//
// Get the output-headpose, so it can be displayed.
//
void Tracker::getOutputHeadPose( THeadPoseData *data ) {
	data->x = output_camera.x;										// centimeters
	data->y = output_camera.y;
	data->z = output_camera.z;

	data->pitch = output_camera.pitch;	// degrees
	data->yaw   = output_camera.yaw;
	data->roll  = output_camera.roll;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void Tracker::loadSettings() {
	qDebug() << "Tracker::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "loadSettings says: iniFile = " << currentFile;

    iniFile.beginGroup ( "Tracking" );
	iniFile.endGroup ();
	iniFile.beginGroup ( "KB_Shortcuts" );
    // Reverse Axis
    useAxisReverse = iniFile.value ( "Enable_ReverseAxis", 0 ).toBool();
    YawAngle4ReverseAxis = iniFile.value ( "RA_Yaw", 40 ).toInt();
    Z_Pos4ReverseAxis = iniFile.value ( "RA_ZPos", 50 ).toInt();
    Z_PosWhenReverseAxis = iniFile.value ( "RA_ToZPos", 80 ).toInt();
    inhibit_rx = iniFile.value("Inhibit_Yaw", false).toBool();
    inhibit_ry = iniFile.value("Inhibit_Pitch", false).toBool();
    inhibit_rz = iniFile.value("Inhibit_Roll", false).toBool();
    inhibit_tx = iniFile.value("Inhibit_X", false).toBool();
    inhibit_ty = iniFile.value("Inhibit_Y", false).toBool();
    inhibit_tz = iniFile.value("Inhibit_Z", false).toBool();
    inhibit_zero = iniFile.value("SetZero", false).toBool(); 
	iniFile.endGroup ();
}

void Tracker::setInvertPitch(bool invert) { GlobalPose->Pitch.invert = invert?-1.0f:1.0f; }
void Tracker::setInvertYaw(bool invert) { GlobalPose->Yaw.invert = invert?-1.0f:+1.0f; }
void Tracker::setInvertRoll(bool invert) { GlobalPose->Roll.invert = invert?-1.0f:+1.0f; }
void Tracker::setInvertX(bool invert) { GlobalPose->X.invert = invert?-1.0f:+1.0f; }
void Tracker::setInvertY(bool invert) { GlobalPose->Y.invert = invert?-1.0f:+1.0f; }
void Tracker::setInvertZ(bool invert) { GlobalPose->Z.invert = invert?-1.0f:+1.0f; }
