/* Copyright (c) 2012-2015 Stanislaw Halik
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

double FTNoIR_Filter::f(double val, const double gains[][2])
{
    for (int i = 0; gains[i][0] >= 0; i++)
    {
        if (val >= gains[i][0])
        {
            return gains[i][1] * val;
        }
    }
    return 0;
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
    
    static const double rot_gains[][2] = {
        { 6, 15 },
        { 5, 6 },
        { 4, 3 },
        { 3, 1.3 },
        { 2, .7 },
        { 1, .4 },
        { 0, .2 },
        { -1, 0 }
    };
    static const double trans_gains[][2] = {
        { 4, 8 },
        { 3, 4 },
        { 2, 2 },
        { 1, .5 },
        { 0, .1 },
        { -1, 0 }
    };

    const double rot_t = 10. * (1+s.rot_threshold) / 100.;
    const double trans_t = 5. * (1+s.trans_threshold) / 100.;
    
    const double dt = t.elapsed() * 1e-9;
    t.start();

    const double RC = 2 * s.ewma / 1000.; // seconds
    const double alpha = dt/(dt+RC);
    const double rot_dz = s.rot_deadzone * 2. / 100.;
    const double trans_dz = s.trans_deadzone * 1. / 100.;
    
    for (int i = 0; i < 6; i++)
    {
        smoothed_input[i] = smoothed_input[i] * (1.-alpha) + input[i] * alpha;
        
        const double in = smoothed_input[i];
        
        const double vec = in - last_output[i];
        const double dz = i >= 3 ? rot_dz : trans_dz;
        const double vec_ = max(0., fabs(vec) - dz);
        const double thres = i >= 3 ? rot_t : trans_t;
        const double val = f(vec_ / thres, i >= 3 ? rot_gains : trans_gains) * thres;
        static Timer tr;
        static double m = 0, n = 0;
        if (i == 3)
        {
            m = max(vec_ / thres, m);
            n = max(n, val * dt);
        }
        if (tr.elapsed_ms() > 1000)
        {
            tr.start();
            qDebug() << "3" << m << n;
            m = 0;
            n = 0;
        }
        const double result = last_output[i] + (vec < 0 ? -1 : 1) * dt * val;
        const bool negp = vec < 0.;
        const bool done = negp
            ? result <= in
            : result >= in;
        if (i == 3 && val > 0.1 && done)
            qDebug() << "done";
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
