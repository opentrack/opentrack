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

FTNoIR_Filter::FTNoIR_Filter() :
    first_run(true),
    b(bundle("Accela")),
    rotation_alpha(b, "rotation-alpha", ACCELA_SMOOTHING_ROTATION),
    translation_alpha(b, "translation-alpha", ACCELA_SMOOTHING_TRANSLATION),
    second_order_alpha(b, "second-order-alpha", ACCELA_SECOND_ORDER_ALPHA),
    third_order_alpha(b, "third-order-alpha", ACCELA_THIRD_ORDER_ALPHA),
    deadzone(b, "deadzone", 0),
    expt(b, "exponent", 2)
{
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
                                       double *new_camera_position)
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
        const double velocity =
                parabola(a, vec, deadzone, expt) +
                parabola(a2, vec2, deadzone, expt) +
                parabola(a3, vec3, deadzone, expt);
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
