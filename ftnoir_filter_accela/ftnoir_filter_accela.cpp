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

FTNoIR_Filter::FTNoIR_Filter() : first_run(true), fast_state { 0,0,0 }
{
}

static inline double dz(double x, double dz)
{
    return std::max(0., fabs(x) - dz) * (x < 0. ? -1. : 1.);
}

void FTNoIR_Filter::filter(const double* input, double *output)
{
    if (first_run)
    {
        for (int i = 0; i < 3; i++)
            fast_state[i] = 0;

        for (int i = 0; i < 6; i++)
        {
            output[i] = input[i];
            for (int j = 0; j < 3; j++)
                last_output[i] = input[i];
        }
        first_run = false;
        return;
    }

    const double rot_dz = s.rot_deadzone;
    const double trans_dz = s.trans_deadzone;

    const int s_rot_plus = s.rot_plus, s_rot_minus = s.rot_minus;

    const double a_rot_plus = s_rot_plus/100.;
    const double a_rot_minus = s_rot_minus/100. * a_rot_plus;
    const double a_trans = s.trans_smoothing/100.;

    static constexpr double fast_alpha = Hz/(Hz + fast_alpha_seconds);

    for (int i = 0; i < 6; i++)
    {
        const double vec = input[i] - last_output[i];
        double datum = Hz * 16;

        if (i >= 3)
        {
            int k = i - 3;
            const double vec_ = dz(vec, rot_dz);
            const double cur_fast = fabs(vec_) * fast_alpha + fast_state[k]*(1. - fast_alpha);
            fast_state[k] = cur_fast;
            const double c = cur_fast > max_slow_delta ? a_rot_plus : a_rot_minus;
            datum *= vec_ * c;
        }
        else
            datum *= dz(vec, trans_dz) * a_trans;

        const double result = last_output[i] + datum;
        const bool negp = vec < 0.;
        const bool done = negp ? result <= input[i] : result >= input[i];

        const double ret = done ? input[i] : result;
        last_output[i] = output[i] = ret;
    }

    state.y = fast_state[0];
    state.p = fast_state[1];
    state.r = fast_state[2];
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_FilterDll;
}
