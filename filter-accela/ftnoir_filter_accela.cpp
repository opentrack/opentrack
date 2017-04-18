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

using std::fabs;
using std::sqrt;
using std::pow;
using std::copysign;

constexpr double settings_accela::rot_gains[16][2];
constexpr double settings_accela::pos_gains[16][2];

accela::accela() : first_run(true)
{
    s.make_splines(spline_rot, spline_pos);
}

template <typename T>
static inline constexpr T signum(T x)
{
    return T((T(0) < x) - (x < T(0)));
}

template<int N = 3, typename F>
static void do_deltas(const double* deltas, double* output, F&& fun)
{
    double norm[N];

    const double dist = progn(
        double ret = 0;
        for (unsigned k = 0; k < N; k++)
            ret += deltas[k]*deltas[k];
        return sqrt(ret);
    );

    const double value = double(fun(dist));

    for (unsigned k = 0; k < N; k++)
    {
        const double c = dist > 1e-6 ? clamp((fabs(deltas[k]) / dist), 0., 1.) : 0;
        norm[k] = c;
    }

    progn(
        double n = 0;
        for (unsigned k = 0; k < N; k++)
            n += norm[k];

        if (n > 1e-6)
        {
            const double ret = 1./n;
            for (unsigned k = 0; k < N; k++)
                norm[k] *= ret;
        }
        else
            for (unsigned k = 0; k < N; k++)
                norm[k] = 0;
    );

    for (unsigned k = 0; k < N; k++)
    {
        const double d = norm[k] * value;
        output[k] = signum(deltas[k]) * d;
    }
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

    const double rot_thres = s.rot_sensitivity.to<double>();
    const double pos_thres = s.pos_sensitivity.to<double>();

    const double dt = t.elapsed_seconds();
    t.start();

    const double RC = s.ewma.to<double>() / 1000.; // seconds
    const double alpha = dt/(dt+RC);
    const double rot_dz = s.rot_deadzone.to<double>();
    const double pos_dz = s.pos_deadzone.to<double>();
    const double nl = s.rot_nonlinearity.to<double>();

    for (unsigned i = 0; i < 6; i++)
        smoothed_input[i] = smoothed_input[i] * (1-alpha) + input[i] * alpha;

    // rot

    for (unsigned i = 3; i < 6; i++)
    {
        double d = smoothed_input[i] - last_output[i];

        if (fabs(d) > rot_dz)
            d -= copysign(rot_dz, d);
        else
            d = 0;

        deltas[i] = d / rot_thres;
    }

    if (nl > 1.)
    {
        for (unsigned k = 3; k < 6; k++)
        {
            static constexpr double nl_end = 1.5;

            if (fabs(deltas[k]) <= nl_end)
                deltas[k] = copysign(pow(fabs(deltas[k]/nl_end), nl) * nl_end, deltas[k]);
        }
    }

    do_deltas(&deltas[Yaw], &output[Yaw], [this](double x) { return spline_rot.get_value_no_save(x); });

    // pos

    for (unsigned i = 0; i < 3; i++)
    {
        double d = smoothed_input[i] - last_output[i];
        if (fabs(d) > pos_dz)
            d -= copysign(pos_dz, d);
        else
            d = 0;

        deltas[i] = d / pos_thres;
    }

    do_deltas(&deltas[TX], &output[TX], [this](double x) { return spline_pos.get_value_no_save(x); });

    // end

    for (unsigned k = 0; k < 6; k++)
    {
        output[k] *= dt;
        output[k] += last_output[k];
        last_output[k] = output[k];
    }
}

void settings_accela::make_splines(spline& rot, spline& trans)
{
    rot = spline();
    trans = spline();

    rot.set_max_input(rot_gains[0][0]);
    trans.set_max_input(pos_gains[0][0]);
    rot.set_max_output(rot_gains[0][1]);
    trans.set_max_output(pos_gains[0][1]);

    for (int i = 0; rot_gains[i][0] >= 0; i++)
        rot.add_point(QPointF(rot_gains[i][0], rot_gains[i][1]));

    for (int i = 0; pos_gains[i][0] >= 0; i++)
        trans.add_point(QPointF(pos_gains[i][0], pos_gains[i][1]));
}

OPENTRACK_DECLARE_FILTER(accela, dialog_accela, accelaDll)

