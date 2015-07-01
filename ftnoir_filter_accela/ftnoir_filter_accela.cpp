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

static constexpr double rot_gains[][2] = {
    { 7, 200 },
    { 6, 100 },
    { 5, 45 },
    { 4, 15 },
    { 3, 5 },
    { 2, 1.4 },
    { 1, .4 },
    { 0, .2 },
    { -1, 0 }
};
static constexpr double trans_gains[][2] = {
    { 5, 180 },
    { 4, 64 },
    { 3, 20 },
    { 2, 5 },
    { 1, .7 },
    { 0, .1 },
    { -1, 0 }
};

FTNoIR_Filter::FTNoIR_Filter() : first_run(true)
{
    rot.setMaxInput(rot_gains[0][0]);
    trans.setMaxInput(trans_gains[0][0]);
    rot.setMaxOutput(rot_gains[0][1]);
    trans.setMaxOutput(trans_gains[0][1]);
    
    for (int i = 0; rot_gains[i][0] >= 0; i++)
    {
        rot.addPoint(QPointF(rot_gains[i][0], rot_gains[i][1]));
    }
    for (int i = 0; trans_gains[i][0] >= 0; i++)
    {
        trans.addPoint(QPointF(trans_gains[i][0], trans_gains[i][1]));
    }
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
        Map& m = i >= 3 ? rot : trans;
        
        smoothed_input[i] = smoothed_input[i] * (1.-alpha) + input[i] * alpha;
        
        const double in = smoothed_input[i];
        
        const double vec = in - last_output[i];
        const double dz = i >= 3 ? rot_dz : trans_dz;
        const double vec_ = max(0., fabs(vec) - dz);
        const double thres = i >= 3 ? rot_t : trans_t;
        const double val = m.getValue(vec_ / thres) * thres;
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
