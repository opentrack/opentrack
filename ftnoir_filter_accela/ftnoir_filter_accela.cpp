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

FTNoIR_Filter::FTNoIR_Filter() : first_run(true)
{
}

double FTNoIR_Filter::f(double vec, double thres)
{
    if (vec > thres*high_thres_c)
        return (vec - thres*high_thres_c) * high_thres_out + thres*high_thres_c;
    if (vec > thres)
        return (vec - thres) * low_thres_mult + thres;
    return pow(vec / thres, 2.0) * thres;
}

void FTNoIR_Filter::filter(const double* input, double *output)
{
    if (first_run)
    {
        for (int i = 0; i < 6; i++)
        {
            output[i] = input[i];
            last_output[i] = input[i];
            smoothed_input[i] = input[i];
        }
        first_run = false;
        t.start();
        return;
    }

    const double rot_t = 10. * s.rot_threshold / 100.;
    const double trans_t = 10. * s.trans_threshold / 100.;

    const double dt = t.elapsed() * 1e-9;
    t.start();

    double RC;

    switch (s.ewma)
    {
    default:
    case 0: // none
        RC = 0;
        break;
    case 1: // low
        RC = 0.09;
        break;
    case 2: // normal
        RC = 0.13;
        break;
    case 3: // high
        RC = 0.16;
        break;
    case 4: // extreme
        RC = 0.19;
        break;
    }
    
    for (int i = 0; i < 6; i++)
    {
        const double alpha = dt/(dt+RC);
        
        smoothed_input[i] = smoothed_input[i] * (1.-alpha) + input[i] * alpha;
        
        const double in = smoothed_input[i];
        
        const double vec = in - last_output[i];
        const double vec_ = fabs(vec);
        const double t = i >= 3 ? rot_t : trans_t;
        const double val = f(vec_, t);
        const double result = last_output[i] + (vec < 0 ? -1 : 1) * dt * val;
        const bool negp = vec < 0.;
        const bool done = negp
            ? result <= in
            : result >= in;
        const double ret = done ? in : result;
        
        last_output[i] = output[i] = ret;
    }
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_FilterDll;
}
