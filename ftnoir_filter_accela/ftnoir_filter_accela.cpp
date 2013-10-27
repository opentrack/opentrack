/* Copyright (c) 2012 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <algorithm>
#include <cmath>
#include <QDebug>
#include <QMutexLocker>
#include "facetracknoir/global-settings.h"

using namespace std;

FTNoIR_Filter::FTNoIR_Filter()
{
	first_run = true;
	loadSettings();
}

FTNoIR_Filter::~FTNoIR_Filter()
{

}

void FTNoIR_Filter::loadSettings() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	iniFile.beginGroup ( "Accela" );
    zoom_factor = iniFile.value("zoom-slowness", ACCELA_ZOOM_SLOWNESS).toDouble();
    rotation_alpha = iniFile.value("rotation-alpha", ACCELA_SMOOTHING_ROTATION).toDouble();
    translation_alpha = iniFile.value("translation-alpha", ACCELA_SMOOTHING_TRANSLATION).toDouble();

    deadzone = iniFile.value("deadzone", 0.0).toDouble();
    // bigger means less filtering
    static const double init_scaling[] = {
        1,   // X
        1,   // Y
        1,   // Z
        1,   // Yaw
        1,   // Pitch
        1    // Roll
    };
    for (int i = 0; i < 6; i++)
    {
        scaling[i] = iniFile.value(QString("axis-%1").arg(QString::number(i)), init_scaling[i]).toDouble();
    }
    expt = iniFile.value("exponent", 2.0).toDouble();

    iniFile.endGroup();
}

void FTNoIR_Filter::receiveSettings(double rot, double trans, double zoom_fac, double dz, double exponent)
{
    QMutexLocker foo(&mutex);
    
    rotation_alpha = rot;
    translation_alpha = trans;
    zoom_factor = zoom_fac;
    deadzone = dz;
    expt = exponent;
}

static inline double parabola(const double a, const double x, const double dz, const double expt)
{
    const double a1 = 1./a;
    return a1 * pow(std::max<double>(x - dz, 1e-3), expt);
}

void FTNoIR_Filter::FilterHeadPoseData(const double* target_camera_position,
                                       double *new_camera_position,
                                       const double* last_post_filter_values)
{
	if (first_run)
	{
        for (int i = 0; i < 6; i++)
        {
            new_camera_position[i] = target_camera_position[i];
            current_camera_position[i] = target_camera_position[i];
        }

		first_run = false;
		return;
	}
    
    QMutexLocker foo(&mutex);

    for (int i=0;i<6;i++)
	{
        const double vec = target_camera_position[i] - current_camera_position[i];
		const int sign = vec < 0 ? -1 : 1;
		const double x = fabs(vec);
        const double a = i >= 3 ? rotation_alpha : translation_alpha;
        const double reduction = 1. / std::max(1., 1. + zoom_factor * -last_post_filter_values[TZ] / 1000);
        const double velocity = parabola(a, x * scaling[i], deadzone, expt) * reduction;
		const double result = current_camera_position[i] + velocity * sign;
        const bool done = sign > 0 ? result >= target_camera_position[i] : result <= target_camera_position[i];
        new_camera_position[i] = current_camera_position[i] = done ? target_camera_position[i] : result;
	}
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}
