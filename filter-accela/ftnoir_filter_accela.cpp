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

#include <QQuaternion>
#include <QVector3D>

accela::accela()
{
    s.make_splines(spline_rot, spline_pos);
}

template<typename F>
never_inline
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
never_inline
static void do_delta(double delta, double* output, F&& fun)
{
    *output = fun(fabs(delta)) * signum(delta);
}

void accela::filter(const double* input, double *output)
{
    static constexpr double full_turn = 360.0;	
    static constexpr double half_turn = 180.0;	

    if (unlikely(first_run))
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
#ifdef ACCELA_USE_EULER_ANGLES
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
        returnspline_rot.get_value_no_save(x);
    });
#endif
    
    for (unsigned i=3; i<6; ++i)
    {
        output[i] = last_output[i] + dt*output[i];
    }

#else
    {
        // Inter/extrapolates along the arc between the old and new orientation.
        // It's basically a quaternion spherical linear interpolation, where the
        // accela gain x dt is the blending parameter. Might actually overshoot
        // the new orientation, but that's fine.

        // Compute rotation angle and axis which brings the previous orientation to the current rotation
        const auto qcurrent = QQuaternion::fromEulerAngles(input[Pitch], input[Yaw], -input[Roll]);
        const auto qlast = QQuaternion::fromEulerAngles(last_output[Pitch], last_output[Yaw], -last_output[Roll]);
        QVector3D axis;
        float angle;
        (qlast.conjugated() * qcurrent).getAxisAndAngle(&axis, &angle);
        // Apply the Accela gain magic. Also need to multiply with dt here.
        angle = std::max(0.f, angle - std::copysign(static_cast<float>(rot_dz), angle)) / static_cast<float>(rot_thres);
        const double gain_angle = dt*spline_rot.get_value_no_save(std::abs(angle)) * signum(angle);
        // Chaining the rotation offset with the axis we just computed, but he angle is now the accela gain.
        const QQuaternion output_quat = qlast * QQuaternion::fromAxisAndAngle(axis, static_cast<float>(gain_angle));
        // And back to Euler angles
        const QVector3D output_euler = output_quat.toEulerAngles();
        output[Pitch] =  output_euler.x();
        output[Yaw]   =  output_euler.y();
        output[Roll]  = -output_euler.z();
    }
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

    for (unsigned i=0; i<3; ++i)
    {
        output[i] = last_output[i] + dt*output[i];
    }

    // end

    for (unsigned k = 0; k < 6; k++)
    {
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
