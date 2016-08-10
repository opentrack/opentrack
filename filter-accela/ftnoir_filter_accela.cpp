/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela.h"
#include <algorithm>
#include <cmath>
#include <QDebug>
#include <QMutexLocker>
#include "opentrack/plugin-api.hpp"

static constexpr double rot_gains[][2] = {
    { 6, 200 },
    { 2.66, 50 },
    { 1.66, 17 },
    { 1, 4 },
    { .5, .53 },
    { 0, 0 },
    { -1, 0 }
};

static constexpr double trans_gains[][2] = {
    { 2.33, 40 },
    { 1.66, 13 },
    { 1.33, 5 },
    { .66, 1 },
    { .33, .5 },
    { 0, 0 },
    { -1, 0 }
};

constexpr double settings_accela::mult_rot;
constexpr double settings_accela::mult_trans;
constexpr double settings_accela::mult_rot_dz;
constexpr double settings_accela::mult_trans_dz;
constexpr double settings_accela::mult_ewma;

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
            const double f = input[i];
            output[i] = f;
            last_output[i] = f;
            smoothed_input[i] = f;
        }
        first_run = false;
        t.start();
        return;
    }

    const double rot_t = (1+s.rot_threshold) * s.mult_rot;
    const double trans_t = (1+s.trans_threshold) * s.mult_trans;

    const double dt = t.elapsed_seconds();
    t.start();

    const double RC = s.mult_ewma * s.ewma / 1000.; // seconds
    const double alpha = dt/(dt+RC);
    const double rot_dz = s.rot_deadzone * s.mult_rot_dz;
    const double trans_dz = s.trans_deadzone * s.mult_trans_dz;
    const double rot_nl = static_cast<const slider_value&>(s.rot_nonlinearity).cur();

    for (int i = 0; i < 6; i++)
    {
        spline& m = i >= 3 ? rot : trans;

        smoothed_input[i] = smoothed_input[i] * (1.-alpha) + input[i] * alpha;

        const double in = smoothed_input[i];

        const double vec = in - last_output[i];
        const double dz = i >= 3 ? rot_dz : trans_dz;
        const double vec_ = std::max(0., fabs(vec) - dz);
        const double thres = i >= 3 ? rot_t : trans_t;
        const double out_ = vec_ / thres;
        const double out = i >= 3 && std::fabs(rot_nl - 1) > 5e-3 && vec_ < s.max_rot_nl
                               ? (std::pow(out_/s.max_rot_nl, rot_nl) * s.max_rot_nl)
                               : out_;
        const double val = m.getValue(float(out));
        last_output[i] = output[i] = last_output[i] + signum(vec) * dt * val;
    }
}

OPENTRACK_DECLARE_FILTER(FTNoIR_Filter, FilterControls, FTNoIR_FilterDll)

