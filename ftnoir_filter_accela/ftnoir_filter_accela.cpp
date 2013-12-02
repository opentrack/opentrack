/* Copyright (c) 2012-2013 Stanislaw Halik
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
    second_order_alpha = iniFile.value("second-order-alpha", ACCELA_SECOND_ORDER_ALPHA).toDouble();
    third_order_alpha = iniFile.value("third-order-alpha", ACCELA_THIRD_ORDER_ALPHA).toDouble();
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
    const double sign = x > 0 ? 1 : -1;
    const double a1 = 1./a;
    return a1 * pow(std::max<double>(fabs(x) - dz, 0), expt) * sign;
}

template<typename T>
static inline T clamp(const T min, const T max, const T value)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
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
            last_input[i] = target_camera_position[i];
            for (int j = 0; j < 3; j++)
                last_output[j][i] = target_camera_position[i];
        }

        timer.start();
        frame_delta = 1;
        first_run = false;
		return;
	}

    bool new_frame = false;

    for (int i = 0; i < 6; i++)
    {
        if (target_camera_position[i] != last_input[i])
        {
            new_frame = true;
            break;
        }
    }

    if (new_frame)
    {
        for (int i = 0; i < 6; i++)
            last_input[i] = target_camera_position[i];
        frame_delta = timer.isValid() ? timer.elapsed() : 1;
        timer.start();
    } else {
        auto d = timer.elapsed();
        double c = clamp(0.0, 1.0, d / (double) frame_delta);
        for (int i = 0; i < 6; i++)
            new_camera_position[i] =
                    last_output[1][i] + (last_output[0][i] - last_output[1][i]) * c;
        return;
    }
    
    QMutexLocker foo(&mutex);

    for (int i=0;i<6;i++)
	{
        const double vec = target_camera_position[i] - last_output[0][i];
        const double vec2 = target_camera_position[i] - last_output[1][i];
        const double vec3 = target_camera_position[i] - last_output[2][i];
		const int sign = vec < 0 ? -1 : 1;
        const double a = i >= 3 ? rotation_alpha : translation_alpha;
        const double a2 = a * second_order_alpha;
        const double a3 = a * third_order_alpha;
        const double reduction = 1. / std::max(1., 1. + zoom_factor * -last_post_filter_values[TZ] / 1000);
        const double velocity =
                parabola(a, vec * scaling[i], deadzone, expt) * reduction +
                parabola(a2, vec2 * scaling[i], deadzone, expt) * reduction +
                parabola(a3, vec3 * scaling[i], deadzone, expt) * reduction;
        const double result = last_output[0][i] + velocity;
        const bool done = sign > 0 ? result >= target_camera_position[i] : result <= target_camera_position[i];
        new_camera_position[i] = done ? target_camera_position[i] : result;
        last_output[2][i] = last_output[1][i];
        last_output[1][i] = last_output[0][i];
        last_output[0][i] = new_camera_position[i];
	}
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}
