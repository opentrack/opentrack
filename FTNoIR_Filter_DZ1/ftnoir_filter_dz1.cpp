/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
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
********************************************************************************/
#include "ftnoir_filter_DZ1.h"
#include "math.h"
#include <QDebug>

//#define LOG_OUTPUT

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FTNoIR_Filter::FTNoIR_Filter()
{
	//populate the description strings
	filterFullName = "Deadzone Filter Mk1";
	filterShortName = "DZ1";
	filterDescription = "Deadzone Filter";

	first_run = true;
	loadSettings();					// Load the Settings

}

FTNoIR_Filter::~FTNoIR_Filter()
{

}

void FTNoIR_Filter::Release()
{
    delete this;
}

void FTNoIR_Filter::Initialize()
{
	qDebug() << "FTNoIR_Filter::Initialize says: Starting ";
	loadSettings();
	return;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Filter::loadSettings() {
	qDebug() << "FTNoIR_Filter::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::loadSettings says: iniFile = " << currentFile;

	//
	// The EWMA2-filter-settings are in the Tracking group: this is because they used to be on the Main Form of FaceTrackNoIR
	//
	iniFile.beginGroup ( "Filter_DZ1" );
	kCameraHz = iniFile.value ( "cameraHz", 30 ).toInt();
	kDeadZone = iniFile.value ( "DeadZone", 0.1f ).toFloat();
	kMoveLast = (iniFile.value ( "MoveLast", 24 ).toFloat()) / 100;					// Convert from int to float percentage
	kMaxDiff = iniFile.value ( "MaxDiff", 1.75f ).toFloat();
	kMoveSaved = ((iniFile.value ( "MoveSaved", 35 ).toFloat()) / 100) / kCameraHz;	// Convert from int to float percentage and divide by Hz
	iniFile.endGroup ();

}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget)
{
	//non-optimised version for clarity
	double target[6];
	double prev_output[6];
	double scale[]={3.0, 3.0, 3.0, 3.0, 3.0, 3.0};
//	static double deadzones[] = { DEADZONE, DEADZONE, DEADZONE, DEADZONE, DEADZONE, DEADZONE };
	float output[6];
	int i=0, j=0;

	//find out how far the head has moved
	prev_output[0]=current_camera_position->x;
	prev_output[1]=current_camera_position->y;
	prev_output[2]=current_camera_position->z;
	prev_output[3]=current_camera_position->yaw;
	prev_output[4]=current_camera_position->pitch;
	prev_output[5]=current_camera_position->roll;

	target[0]=target_camera_position->x;
	target[1]=target_camera_position->y;
	target[2]=target_camera_position->z;
	target[3]=target_camera_position->yaw;
	target[4]=target_camera_position->pitch;
	target[5]=target_camera_position->roll;

	if (first_run)
	{
		//on the first run, output=target
		for (i=0;i<6;i++)
		{
			last_positions[i] = target[i];
			saved_positions[i] = target[i];
			prev_positions[i] = target[i];
			smooth_remember[i] = 0;
			smoothing[i] = false;
			smooth_speed[i] = -1.0;
		}

		new_camera_position->x=target[0];
		new_camera_position->y=target[1];
		new_camera_position->z=target[2];
		new_camera_position->yaw=target[3];
		new_camera_position->pitch=target[4];
		new_camera_position->roll=target[5];

		first_run=false;
		
		//we can bail
		return;
	} else if (!newTarget) {
		int i;
		for (i = 0; i < 6; i++) {
			output[i] = prev_positions[i];
		}
		goto end;
	}

	for (i=0;i<6;i++)
	{
		double e2 = target[i];
		bool slowp = fabs(last_positions[i] - e2) < kDeadZone && fabs(e2 - saved_positions[i]) < kMaxDiff;		// DeadZone and MAXDIFF from INI-file
		if (smoothing[i] || !slowp) {
			double start = smooth_start[i];
			double vec = e2 - start;
			int sign = vec < 0 ? -1 : 1;
			int sign2 = e2 < 0 ? -1 : 1;
			if (sign != sign2)
				sign2 = -sign2;
			double diff = (kCameraHz / REMEMBER_SMOOTHNESS) - smooth_remember[i];
			smooth_speed[i] *= -(diff * diff) / (double) ((kCameraHz / REMEMBER_SMOOTHNESS) * (kCameraHz / REMEMBER_SMOOTHNESS)) + 1.001;
			if (smooth_speed[i] < INITIAL_SMOOTH_SPEED)
				smooth_speed[i] = INITIAL_SMOOTH_SPEED;
			vec = fabs(vec);
			double iter = vec * SMOOTH_FACTOR / (double) kCameraHz;
			smooth_speed[i] += iter;
			double bleh = smooth_speed[i] * sign;
			double foo = smooth_start[i] + bleh;
			bool done = sign2 > 0 ? foo > e2 : foo < e2;
			smooth_remember[i] = (kCameraHz / REMEMBER_SMOOTHNESS);
			if (done) {
				smoothing[i] = false;
				output[i] = e2;
				saved_positions[i] = e2;
				last_positions[i] = e2;
				prev_positions[i] = e2;
			} else {
				smoothing[i] = true;
				prev_positions[i] = output[i] = foo;
			}
		} else {
			if (smooth_remember[i] <= 0) {
				smooth_speed[i] = -1.0;
			} else {
				smooth_remember[i]--;
			}
			last_positions[i] = last_positions[i] + (e2 - last_positions[i]) * kMoveLast;							// MOVE_LAST from INI-file
			saved_positions[i] = saved_positions[i] + (e2 - saved_positions[i]) * kMoveSaved;						// MOVE_SAVED from INI-file
			output[i] = prev_positions[i] = prev_positions[i] + (e2 - prev_positions[i]) * (SLOW_SPEED/kCameraHz);	// Get CameraHz from INI
			smooth_start[i] = output[i];
			smoothing[i] = false;
#if 0
			if (i == 3) {
				FILE* dlog = fopen("debug-log.txt", "a");
				fprintf(dlog, "slowed=%f frames=%d diff=%f\n", output[i], slow_move_count[i], e1 - last_positions[i]);
				fclose(dlog);
			}
#endif
		}
	}

end:
	new_camera_position->x=output[0];
	new_camera_position->y=output[1];
	new_camera_position->z=output[2];
	new_camera_position->yaw=output[3] * MULT_X;
	new_camera_position->pitch=output[4] * (output[4] < 0 ? MULT_Y_NEG : MULT_Y_POS) - COCKPIT_PITCH;
	new_camera_position->roll=output[5];

#if 1
	//
	// Also update the 'current' position, for the next iteration.
	//
	current_camera_position->x=output[0];
	current_camera_position->y=output[1];
	current_camera_position->z=output[2];
	current_camera_position->yaw=output[3] * MULT_X;
	current_camera_position->pitch=output[4] * (output[4] < 0 ? MULT_Y_NEG : MULT_Y_POS) + COCKPIT_PITCH;
	current_camera_position->roll=output[5];
#endif
}

void FTNoIR_Filter::getFilterFullName(QString *strToBeFilled)
{
	*strToBeFilled = filterFullName;
};


void FTNoIR_Filter::getFilterShortName(QString *strToBeFilled)
{
	*strToBeFilled = filterShortName;
};


void FTNoIR_Filter::getFilterDescription(QString *strToBeFilled)
{
	*strToBeFilled = filterDescription;
};

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter object.

// Export both decorated and undecorated names.
//   GetFilter     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetFilter@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetFilter=_GetFilter@0")

FTNOIR_FILTER_BASE_EXPORT FILTERHANDLE __stdcall GetFilter()
{
	return new FTNoIR_Filter;
}
