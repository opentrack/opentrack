/* Copyright (c) 2012-2016 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela.h"
#include "compat/math.hpp"
#include "api/plugin-api.hpp"
#include "opentrack/defs.hpp"

#include <algorithm>
#include <QDebug>
#include <QMutexLocker>

#include "compat/math-imports.hpp"
#include "compat/time.hpp"

accela::accela()
{
    s.make_splines(spline_rot, spline_pos);
}

template<typename F>
tr_never_inline
static void do_deltas(const double* deltas, double* output, F&& fun)
{
    constexpr unsigned N = 3;

    double norm[N];
    double dist = 0;

    for (unsigned k = 0; k < N; k++)
        dist += deltas[k]*deltas[k];
    dist = sqrt(dist);

    const double value = fun(dist);

    for (unsigned k = 0; k < N; k++)
    {
        const double c = dist > 1e-6 ? std::clamp((fabs(deltas[k]) / dist), 0., 1.) : 0;
        norm[k] = c;
    }

    double n = 0;
    for (unsigned k = 0; k < N; k++) // NOLINT(modernize-loop-convert)
        n += norm[k];

    if (n > 1e-6)
    {
        const double ret = 1./n;
        for (unsigned k = 0; k < N; k++) // NOLINT(modernize-loop-convert)
            norm[k] *= ret;
    }
    else
        for (unsigned k = 0; k < N; k++) // NOLINT(modernize-loop-convert)
            norm[k] = 0;

    for (unsigned k = 0; k < N; k++)
    {
        const double d = norm[k] * value;
        output[k] = signum(deltas[k]) * d;
    }
}

template<typename F>
[[maybe_unused]]
tr_never_inline
static void do_delta(double delta, double* output, F&& fun)
{
    *output = fun(fabs(delta)) * signum(delta);
}

void accela::filter(const double* input, double *output)
{
    static constexpr double full_turn = 360.0;	
    static constexpr double half_turn = 180.0;	

    if (first_run) [[unlikely]]
    {
        first_run = false;

        for (int i = 0; i < 6; i++)
        {
            const double f = input[i];
            output[i] = f;
            last_output[i] = f;
        }

        t.start();
#if defined DEBUG_ACCELA
        debug_max = 0;
        debug_timer.start();
#endif

        return;
    }

    const double rot_thres{s.rot_smoothing};
    const double pos_thres{s.pos_smoothing};

    const double dt = t.elapsed_seconds();
    t.start();

    const double rot_dz{ s.rot_deadzone};
    const double pos_dz{ s.pos_deadzone};

    // rot

    for (unsigned i = 3; i < 6; i++)
    {
        double d = input[i] - last_output[i];
        if (fabs(d) > half_turn) d -= copysign(full_turn, d);

        if (fabs(d) > rot_dz)
            d -= copysign(rot_dz, d);
        else
            d = 0;

        deltas[i] = d / rot_thres;
    }

#ifdef UI_ACCELA_OLD_STAIRCASE
    for (int i = Yaw; i <= Roll; i++)
        do_delta(deltas[i], &output[i], [this](double x) {
          return spline_rot.get_value_no_save(x);
        });
#else
    do_deltas(&deltas[Yaw], &output[Yaw], [this](double x) {
        return spline_rot.get_value_no_save(x);
    });
#endif

    // pos

    for (unsigned i = 0; i < 3; i++)
    {
        double d = input[i] - last_output[i];
        if (fabs(d) > pos_dz)
            d -= copysign(pos_dz, d);
        else
            d = 0;

        deltas[i] = d / pos_thres;
    }

    do_deltas(&deltas[TX], &output[TX], [this](double x) {
        return spline_pos.get_value_no_save(x);
    });

    // end

    for (unsigned k = 0; k < 6; k++)
    {
        output[k] *= dt;
        output[k] += last_output[k];
        if (k >= Yaw && fabs(output[k]) > half_turn)
            output[k] -= copysign(full_turn, output[k]);

        last_output[k] = output[k];
    }
}

namespace detail::accela {

void settings_accela::make_splines(spline& rot, spline& pos)
{
    rot.clear(); pos.clear();

    for (const auto& val : rot_gains)
        rot.add_point({ val.x, val.y });

    for (const auto& val : pos_gains)
        pos.add_point({ val.x, val.y });
}

} // ns detail::accela

OPENTRACK_DECLARE_FILTER(accela, dialog_accela, accelaDll)
