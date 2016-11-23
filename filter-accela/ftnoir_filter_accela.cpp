/* Copyright (c) 2012-2016 Stanislaw Halik
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
#include "api/plugin-api.hpp"

constexpr double settings_accela::rot_gains[16][2];
constexpr double settings_accela::trans_gains[16][2];

accela::accela() : first_run(true)
{
    s.make_splines(rot, trans);
}

void accela::filter(const double* input, double *output)
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

#define cast(x) (static_cast<slider_value&&>((x)))

    const double rot_t = cast(s.rot_sensitivity).cur();
    const double trans_t = cast(s.trans_sensitivity).cur();

    const double dt = t.elapsed_seconds();
    t.start();

    const double RC = cast(s.ewma).cur() / 1000.; // seconds
    const double alpha = dt/(dt+RC);
    const double rot_dz = cast(s.rot_deadzone).cur();
    const double trans_dz = cast(s.trans_deadzone).cur();
    const slider_value nl = s.rot_nonlinearity;

    for (int i = 0; i < 6; i++)
    {
        spline& m = i >= 3 ? rot : trans;

        smoothed_input[i] = smoothed_input[i] * (1-alpha) + input[i] * alpha;

        const double in = smoothed_input[i];

        const double vec_ = in - last_output[i];
        const double dz = i >= 3 ? rot_dz : trans_dz;
        const double vec = std::max(0., fabs(vec_) - dz);
        const double thres = i >= 3 ? rot_t : trans_t;
        const double out_ = vec / thres;
        const double out = progn(
                               const bool should_apply_rot_nonlinearity =
                                   i >= 3 &&
                                   std::fabs(nl.cur() - 1) > 5e-3 &&
                                   vec < nl.max();

                               if (should_apply_rot_nonlinearity)
                                   return std::pow(out_/nl.max(), nl.cur()) * nl.max();
                               else
                                   return out_;
        );
        const double val = double(m.getValue(out));
        last_output[i] = output[i] = last_output[i] + signum(vec_) * dt * val;
    }
}



void settings_accela::make_splines(spline& rot, spline& trans)
{
    rot = spline();
    trans = spline();

    rot.setMaxInput(rot_gains[0][0]);
    trans.setMaxInput(trans_gains[0][0]);
    rot.setMaxOutput(rot_gains[0][1]);
    trans.setMaxOutput(trans_gains[0][1]);

    for (int i = 0; rot_gains[i][0] >= 0; i++)
        rot.addPoint(QPointF(rot_gains[i][0], rot_gains[i][1]));

    for (int i = 0; trans_gains[i][0] >= 0; i++)
        trans.addPoint(QPointF(trans_gains[i][0], trans_gains[i][1]));
}

OPENTRACK_DECLARE_FILTER(accela, dialog_accela, accelaDll)

