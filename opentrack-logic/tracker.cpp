/* Copyright (c) 2012-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

/*
 * this file appeared originally in facetracknoir, was rewritten completely
 * following opentrack fork.
 *
 * originally written by Wim Vriend.
 */


#include "tracker.h"
#include <cmath>
#include <algorithm>

#if defined(_WIN32)
#   include <windows.h>
#endif

Tracker::Tracker(Mappings &m, SelectedLibraries &libs, TrackLogger &logger) :
    m(m),
    newpose {0,0,0, 0,0,0},
    centerp(s.center_at_startup),
    enabledp(true),
    zero_(false),
    should_quit(false),
    libs(libs),
    logger(logger),
    r_b(get_camera_offset_matrix(c_div).t()),
    r_b_real(get_camera_offset_matrix(1).t()),
    t_b {0,0,0}
{
}

Tracker::~Tracker()
{
    should_quit = true;
    wait();
}

Tracker::rmat Tracker::get_camera_offset_matrix(double c)
{
    const double off[] =
    {
        d2r * c * (double)-s.camera_yaw,
        d2r * c * (double)-s.camera_pitch,
        d2r * c * (double)-s.camera_roll
    };

    return euler::euler_to_rmat(off);
}

double Tracker::map(double pos, Map& axis)
{
    bool altp = (pos < 0) && axis.opts.altp;
    axis.spline_main.setTrackingActive( !altp );
    axis.spline_alt.setTrackingActive( altp );
    auto& fc = altp ? axis.spline_alt : axis.spline_main;
    return fc.getValue(pos);
}

void Tracker::t_compensate(const rmat& rmat, const euler_t& xyz_, euler_t& output, bool rz)
{
    // TY is really yaw axis. need swapping accordingly.
    const euler_t ret = rmat * euler_t(xyz_(TZ), -xyz_(TX), -xyz_(TY));
    if (!rz)
        output(2) = ret(0);
    else
        output(2) = xyz_(2);
    output(1) = -ret(2);
    output(0) = -ret(1);
}

#include "opentrack-compat/nan.hpp"

static inline double elide_nan(double value, double def)
{
    if (nanp(value))
    {
        if (nanp(def))
            return 0;
        return def;
    }
    return value;
}

static bool is_nan(const dmat<3,3>& r, const dmat<3, 1>& t)
{
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (nanp(r(i, j)))
                return true;

    for (int i = 0; i < 3; i++)
        if (nanp(t(i)))
            return true;

    return false;
}

static bool is_nan(const Pose& value)
{
    for (int i = 0; i < 6; i++)
        if (nanp(value(i)))
            return true;
    return false;
}

constexpr double Tracker::c_mult;
constexpr double Tracker::c_div;

void Tracker::logic()
{
    using namespace euler;

    Pose value, raw;

    for (int i = 0; i < 6; i++)
    {
        auto& axis = m(i);
        int k = axis.opts.src;
        if (k < 0 || k >= 6)
            value(i) = 0;
        else
            value(i) = newpose[k];
        raw(i) = newpose[i];
    }

    logger.write_pose(raw); // raw

    if (is_nan(raw))
        raw = last_raw;

    // TODO fix gimbal lock by dividing euler angle input by >=3.
    // maintain the real rmat separately for translation compensation
    // TODO split this function, it's too big
    rmat r, r_real;

    {
        euler_t tmp = d2r * euler_t(&value[Yaw]);
        r = euler_to_rmat(c_div * tmp);
        r_real = euler_to_rmat(tmp);
    }

    euler_t t(value(0), value(1), value(2));

    bool do_center_now = false;
    bool nan = is_nan(r, t);

    if (centerp && !nan)
    {
        for (int i = 0; i < 6; i++)
            if (fabs(newpose[i]) != 0)
            {
                do_center_now = true;
                break;
            }
    }

    const rmat cam = get_camera_offset_matrix(c_div);
    const rmat cam_real = get_camera_offset_matrix(1);

    r = r * cam;
    r_real = r_real * cam_real;

    if (do_center_now)
    {
        centerp = false;

        if (libs.pFilter)
            libs.pFilter->center();

        if (libs.pTracker->center())
        {
            r_b = cam.t();
            r_b_real = cam_real.t();
            r = rmat::eye();
            r_real = rmat::eye();
        }
        else
        {
            r_b = r.t();
            r_b_real = r_real.t();
        }

        for (int i = 0; i < 3; i++)
            t_b[i] = t(i);
    }

    {
        switch (s.center_method)
        {
        // inertial
        case 0:
        default:
            r = r_b * r;
            break;
        // camera
        case 1:
            r = r * r_b;
            break;
        }

        const euler_t rot = r2d * c_mult * rmat_to_euler(r);
        euler_t pos(t(0) - t_b[0], t(1) - t_b[1], t(2) - t_b[2]);

        if (s.use_camera_offset_from_centering)
            t_compensate(r_b_real.t() * cam_real.t(), pos, pos, false);
        else
            t_compensate(cam_real.t(), pos, pos, false);

        for (int i = 0; i < 3; i++)
        {
            value(i) = pos(i);
            value(i+3) = rot(i);
        }
    }

    logger.write_pose(value); // "corrected" - after various transformations to account for camera position

    // whenever something can corrupt its internal state due to nan/inf, elide the call
    if (is_nan(value))
    {
        nan = true;
        logger.write_pose(value); // "filtered"
    }
    else
    {
        {
            Pose tmp = value;

            if (libs.pFilter)
                libs.pFilter->filter(tmp, value);
        }
        logger.write_pose(value); // "filtered"

        // CAVEAT rotation only, due to tcomp
        for (int i = 3; i < 6; i++)
            value(i) = map(value(i), m(i));

        for (int i = 0; i < 6; i++)
            value(i) += m(i).opts.zero;

        for (int i = 0; i < 6; i++)
            value(i) *= int(m(i).opts.invert) * -2 + 1;

        if (zero_)
            for (int i = 0; i < 6; i++)
                value(i) = 0;

        if (is_nan(value))
            nan = true;
    }

    if (s.tcomp_p)
    {
        euler_t value_(value(TX), value(TY), value(TZ));
        t_compensate(euler_to_rmat(euler_t(value(Yaw) * d2r, value(Pitch) * d2r, value(Roll) * d2r)),
                     value_,
                     value_,
                     s.tcomp_tz);
        if (is_nan(r, value_))
            nan = true;
        for (int i = 0; i < 3; i++)
            value(i) = value_(i);
    }

    // CAVEAT translation only, due to tcomp
    for (int i = 0; i < 3; i++)
        value(i) = map(value(i), m(i));

    logger.write_pose(value); // "mapped"

    if (nan)
    {
        value = last_mapped;

        // for widget last value display
        for (int i = 0; i < 6; i++)
            (void) map(value(i), m(i));
    }

    logger.next_line();

    libs.pProtocol->pose(value);

    last_mapped = value;
    last_raw = raw;

    QMutexLocker foo(&mtx);
    output_pose = value;
    raw_6dof = raw;
}

void Tracker::run()
{
    const int sleep_ms = 3;

#if defined(_WIN32)
    (void) timeBeginPeriod(1);
#endif

    {
        static constexpr const char* posechannels[6] = { "TX", "TY", "TZ", "Yaw", "Pitch", "Roll" };
        static constexpr const char* datachannels[5] = { "dt", "raw", "corrected", "filtered", "mapped" };
        logger.write(datachannels[0]);
        char buffer[128];
        for (unsigned j = 1; j < 5; ++j)
        {
            for (unsigned i = 0; i < 6; ++i)
            {
                snprintf(buffer, 128, "%s%s", datachannels[j], posechannels[i]);
                logger.write(buffer);
            }
        }
    }
    logger.next_line();

    while (!should_quit)
    {
        {
            double dt = t.elapsed_seconds();
            logger.write(&dt, 1);
        }
        t.start();

        double tmp[6] {0,0,0, 0,0,0};
        libs.pTracker->data(tmp);

        if (enabledp)
            for (int i = 0; i < 6; i++)
                newpose[i] = elide_nan(tmp[i], newpose[i]);

        logic();

        long q = long(sleep_ms * 1000L - t.elapsed()/1000L);
        using std::max;
        using ulong = unsigned long;
        usleep(ulong(max(1L, q)));
    }

    {
        // filter may inhibit exact origin
        Pose p;
        libs.pProtocol->pose(p);
    }

#if defined(_WIN32)
    (void) timeEndPeriod(1);
#endif

    for (int i = 0; i < 6; i++)
    {
        m(i).spline_main.setTrackingActive(false);
        m(i).spline_alt.setTrackingActive(false);
    }
}

void Tracker::get_raw_and_mapped_poses(double* mapped, double* raw) const
{
    QMutexLocker foo(&const_cast<Tracker&>(*this).mtx);

    for (int i = 0; i < 6; i++)
    {
        raw[i] = raw_6dof(i);
        mapped[i] = output_pose(i);
    }
}

