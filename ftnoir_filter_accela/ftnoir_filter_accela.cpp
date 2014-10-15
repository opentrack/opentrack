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
#include "facetracknoir/plugin-support.h"
using namespace std;

FTNoIR_Filter::FTNoIR_Filter() : first_run(true)
{
}

static inline double parabola(const double a, const double x, const double dz, const double expt)
{
    const double sign = x > 0 ? 1 : -1;
    const double a1 = 1./a;
    return a1 * pow(std::max<double>(fabs(x) - dz, 0), expt) * sign;
}

void FTNoIR_Filter::FilterHeadPoseData(const double* target_camera_position,
                                       double *new_camera_position)
{
	if (first_run)
	{
        for (int i = 0; i < 6; i++)
        {
            new_camera_position[i] = target_camera_position[i];
            for (int j = 0; j < 3; j++)
                last_output[j][i] = target_camera_position[i];
        }

        first_run = false;
		return;
	}

    for (int i=0;i<6;i++)
	{
        const double vec = target_camera_position[i] - last_output[0][i];
        const double vec2 = target_camera_position[i] - last_output[1][i];
        const double vec3 = target_camera_position[i] - last_output[2][i];
		const int sign = vec < 0 ? -1 : 1;
        const double a = i >= 3 ? s.rotation_alpha : s.translation_alpha;
        const double a2 = a * s.second_order_alpha;
        const double a3 = a * s.third_order_alpha;
        const double deadzone = i >= 3 ? s.rot_deadzone : s.trans_deadzone;
        const double velocity =
                parabola(a, vec, deadzone, s.expt) +
                parabola(a2, vec2, deadzone, s.expt) +
                parabola(a3, vec3, deadzone, s.expt);
        const double result = last_output[0][i] + velocity;
        const bool done = sign > 0 ? result >= target_camera_position[i] : result <= target_camera_position[i];
        last_output[2][i] = last_output[1][i];
        last_output[1][i] = last_output[0][i];
        last_output[0][i] = new_camera_position[i] = done ? target_camera_position[i] : result;
	}
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}
