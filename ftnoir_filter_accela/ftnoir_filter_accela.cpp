/* Copyright (c) 2012 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
/*
	Modifications (last one on top):
		20120807 - WVR: FunctionConfig is now also used for the Filter. The extrapolation was adapted from Stanislaw.
					    Additional changes: I have added two parameters to the constructor of FunctionConfig and
						renamed 3 member-functions (getFilterFullName is now called getFullName).
*/
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include "math.h"
#include <QDebug>
#include <float.h>
#include "facetracknoir/global-settings.h"

#if !defined(_WIN32) && !defined(__WIN32)
#   define _isnan isnan
#endif

FTNoIR_Filter::FTNoIR_Filter() :
    functionConfig("Accela-Scaling-Rotation", 10, 10),
    translationFunctionConfig("Accela-Scaling-Translation", 10, 10)
{
	first_run = true;
	kMagicNumber = 1000;
	loadSettings();					// Load the Settings
}

FTNoIR_Filter::~FTNoIR_Filter()
{

}

void FTNoIR_Filter::loadSettings() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    functionConfig.loadSettings(iniFile);
    translationFunctionConfig.loadSettings(iniFile);

	iniFile.beginGroup ( "Accela" );
	kMagicNumber = iniFile.value ( "Reduction", 1000 ).toFloat();
    kZoomSlowness = iniFile.value("zoom-slowness", 0).toFloat();
	iniFile.endGroup ();
}

void FTNoIR_Filter::FilterHeadPoseData(double *current_camera_position,
                                       double *target_camera_position,
                                       double *new_camera_position,
                                       double *last_post_filter_values)
{
	double target[6];
	double prev_output[6];
	float output[6];

    for (int i = 0; i < 6; i++)
    {
        prev_output[i] = current_camera_position[i];
        target[i] = target_camera_position[i];
    }

	if (first_run)
	{
        for (int i = 0; i < 6; i++)
        {
            new_camera_position[i] = target[i];
            current_camera_position[i] = target[i];
        }

		first_run=false;
		return;
	}

    for (int i=0;i<6;i++)
	{
		if (_isnan(target[i]))
			return;

		if (_isnan(prev_output[i]))
			return;

		double e2 = target[i];
		double start = prev_output[i];
		double vec = e2 - start;
		int sign = vec < 0 ? -1 : 1;
		double x = fabs(vec);
		QList<QPointF> points = (i >= 3 ? functionConfig : translationFunctionConfig).getPoints();
		int extrapolatep = 0;
		double ratio;
		double maxx;
		double add;
		// linear extrapolation of a spline
		if (points.size() > 1) {
			QPointF last = points[points.size() - 1];
			QPointF penultimate = points[points.size() - 2];
			ratio = (last.y() - penultimate.y()) / (last.x() - penultimate.x());
			extrapolatep = 1;
			add = last.y();
			maxx = last.x();
		}
		double foo = extrapolatep && x > maxx ? add + ratio * (x - maxx) : (i >= 3 ? functionConfig : translationFunctionConfig).getValue(x);
		// the idea is that "empty" updates without new head pose data are still
		// useful for filtering, as skipping them would result in jerky output.
		// the magic "100" is the amount of calls to the filter by FTNOIR per sec.
		// WVR: Added kMagicNumber for Patrick
        double velocity = foo / kMagicNumber * (1 / std::max(1.0, 1 + kZoomSlowness * -last_post_filter_values[TZ] / 100));
		double sum = start + velocity * sign;
		bool done = (sign > 0 ? sum >= e2 : sum <= e2);
		if (done) {
			output[i] = e2;
		} else {
			output[i] = sum;
		}

		if (_isnan(output[i]))
			return;
	}

    for (int i = 0; i < 6; i++)
    {
        new_camera_position[i] = output[i];
        current_camera_position[i] = output[i];
    }
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter object.

// Export both decorated and undecorated names.
//   GetFilter     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetFilter@0  - Common name decoration for __stdcall functions in C language.

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}
