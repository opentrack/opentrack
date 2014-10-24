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
#include "opentrack/plugin-api.hpp"
using namespace std;

FTNoIR_Filter::FTNoIR_Filter() : first_run(true), fast_state { 0,0,0, 0,0,0 }
{
}

static inline double parabola(const double a, const double x, const double dz, const double expt)
{
    const double sign = x > 0 ? 1 : -1;
    const double a1 = 1./a;
    return a1 * pow(std::max(0., fabs(x) - dz), expt) * sign;
}

void FTNoIR_Filter::filter(const double* input, double *output)
{
    if (first_run)
    {
        for (int i = 0; i < 6; i++)
        {
            fast_state[i] = 0;
            output[i] = input[i];
            for (int j = 0; j < 3; j++)
                last_output[i] = input[i];
        }
        first_run = false;
        return;
    }

    const double fast_c = s.fast_alpha / 100.;
    const double rot_dz = s.rot_deadzone;
    const double trans_dz = s.trans_deadzone;
    const double rot_a = s.rotation_alpha;
    const double trans_a = s.translation_alpha;
    const double expt = s.expt;

    static constexpr double fast_alpha = Hz/(Hz + fast_alpha_seconds);

    for (int i=0;i<6;i++)
    {
        const double vec = input[i] - last_output[i];
        const double a = i >= 3 ? rot_a : trans_a;
        const double deadzone = i >= 3 ? rot_dz : trans_dz;

        double datum;

        if (i >= 3)
        {
            const double cur_fast = std::abs(vec) * fast_alpha + fast_state[i]*(1. - fast_alpha);
            fast_state[i] = cur_fast;
            const double how_fast = std::max(0., fast_c * (cur_fast - max_slow_delta));
            datum = parabola(a, vec * (1.-damping + how_fast), deadzone, s.expt);
        }
        else
            datum = parabola(a, vec, deadzone, expt);

        const double result = last_output[i] + datum;
        const bool negp = vec < 0.;
        const bool done = negp ? result <= input[i] : result >= input[i];

        last_output[i] = last_output[i];
        last_output[i] = last_output[i];
        const double ret = done ? input[i] : result;
        last_output[i] = output[i] = ret;
    }

    state.y = output[Yaw] - input[Yaw];
    state.p = output[Pitch] - input[Pitch];
    state.r = output[Roll] - input[Roll];
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_FilterDll;
}
