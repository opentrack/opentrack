/* Copyright (c) 2012 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_Accela.h"
#include "math.h"
#include <QDebug>

FTNoIR_Filter::FTNoIR_Filter()
{
	//populate the description strings
	filterFullName = "Accela Filter";
	filterShortName = "Accela";
	filterDescription = "Accela filter";

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
	loadSettings();
	return;
}

void FTNoIR_Filter::loadSettings() {
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "Filter_Accela" );
	kFactor = iniFile.value ( "factor", 8.0 ).toDouble();
	kSensitivity = iniFile.value("sensitivity", 12.0).toDouble();
	kC0 = iniFile.value("c0", 0.005).toDouble();
	kC1 = iniFile.value("c1", 0.125).toDouble();
	kC2 = iniFile.value("c2", 0.33333).toDouble();
	kC3 = iniFile.value("c3", 0.75).toDouble();
	kC4 = iniFile.value("c4", 1.0).toDouble();
	kFactorTranslation = iniFile.value("factor_translation", 1.0).toDouble();
	kSensitivityTranslation = iniFile.value("sensitivity_translation", 1.0).toDouble();

	iniFile.endGroup ();
}

void FTNoIR_Filter::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget)
{
	double target[6];
	double prev_output[6];
	float output[6];
	int i=0;

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
		new_camera_position->x=target[0];
		new_camera_position->y=target[1];
		new_camera_position->z=target[2];
		new_camera_position->yaw=target[3];
		new_camera_position->pitch=target[4];
		new_camera_position->roll=target[5];

		first_run=false;
		return;
	}

	for (i=0;i<6;i++)
	{
		double e2 = target[i];
		double start = prev_output[i];
		double vec = e2 - start;
		int sign = vec < 0 ? -1 : 1;
		double max = i >= 3 ? kFactor : kFactorTranslation;
		double x = fabs(vec) / max;
		double foo = kC4 * x * x * x * x + kC3 * x * x * x + kC2 * x * x + kC1 * x + kC0;
		// the idea is that "empty" updates without new head pose data are still
		// useful for filtering, as skipping them would result in jerky output.
		// the magic "100" is the amount of calls to the filter by FTNOIR per sec.
		double velocity = foo * (i >= 3 ? kSensitivity : kSensitivityTranslation) / 100.0;
		double sum = start + velocity * sign;
		bool done = sign > 0 ? sum >= e2 : sum <= e2;
		if (done) {
			output[i] = e2;
		} else {
			output[i] = sum;
		}
	}

end:
	new_camera_position->x=output[0];
	new_camera_position->y=output[1];
	new_camera_position->z=output[2];
	new_camera_position->yaw=output[3];
	new_camera_position->pitch=output[4];
	new_camera_position->roll=output[5];

	current_camera_position->x=output[0];
	current_camera_position->y=output[1];
	current_camera_position->z=output[2];
	current_camera_position->yaw=output[3];
	current_camera_position->pitch=output[4];
	current_camera_position->roll=output[5];
}

void FTNoIR_Filter::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = filterFullName;
};


void FTNoIR_Filter::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = filterShortName;
};


void FTNoIR_Filter::getDescription(QString *strToBeFilled)
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
