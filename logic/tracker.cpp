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

#include "compat/sleep.hpp"
#include "compat/util.hpp"

#include "tracker.h"

#include <cmath>
#include <algorithm>
#include <cstdio>

#if defined(_WIN32)
#   include <windows.h>
#endif

using namespace euler;

constexpr double Tracker::r2d;
constexpr double Tracker::d2r;

Tracker::Tracker(Mappings& m, SelectedLibraries& libs, TrackLogger& logger) :
    m(m),
    libs(libs),
    logger(logger),
    backlog_time(0)
{
    set(f_center, s.center_at_startup);
}

Tracker::~Tracker()
{
    set(f_should_quit, true);
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
    return double(fc.getValue(pos));
}

void Tracker::t_compensate(const rmat& rmat, const euler_t& xyz, euler_t& output,
                           bool disable_tx, bool disable_ty, bool disable_tz)
{
    enum { tb_Z, tb_X, tb_Y };

    // TY is really yaw axis. need swapping accordingly.
    // sign changes are due to right-vs-left handedness of coordinate system used
    const euler_t ret = rmat * euler_t(xyz(TZ), -xyz(TX), -xyz(TY));

    if (disable_tz)
        output(TZ) = xyz(TZ);
    else
        output(TZ) = ret(tb_Z);

    if (disable_ty)
        output(TY) = xyz(TY);
    else
        output(TY) = -ret(tb_Y);

    if (disable_tx)
        output(TX) = xyz(TX);
    else
        output(TX) = -ret(tb_X);
}

#include "compat/nan.hpp"

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

template<int u, int w>
static bool is_nan(const dmat<u,w>& r)
{
    for (int i = 0; i < u; i++)
        for (int j = 0; j < w; j++)
            if (nanp(r(u, w)))
                return true;

    return false;
}

constexpr double Tracker::c_mult;
constexpr double Tracker::c_div;

void Tracker::logic()
{
    using namespace euler;

    logger.write_dt();
    logger.reset_dt();

    Pose value, raw;

    for (int i = 0; i < 6; i++)
    {
        auto& axis = m(i);
        int k = axis.opts.src;
        if (k < 0 || k >= 6)
            value(i) = 0;
        else
            value(i) = newpose(k);
        raw(i) = newpose(i);
    }

    // hatire, udp, and freepie trackers can mess up here
    for (unsigned i = 3; i < 6; i++)
    {
        using std::fmod;
        using std::copysign;
        using std::fabs;

        const double x = value(i);
        if (fabs(x) - 1e-2 > 180)
            value(i) = fmod(x - copysign(180, -x), 360) + copysign(180, x);
        else
            value(i) = clamp(x, -180, 180);
    }

    logger.write_pose(raw); // raw

    if (is_nan(raw))
        raw = last_raw;

    // TODO split this function, it's too big

    {
        euler_t tmp = d2r * euler_t(&value[Yaw]);
        scaled_rotation.rotation = euler_to_rmat(c_div * tmp);
        real_rotation.rotation = euler_to_rmat(tmp);
        tait_bryan_to_matrices(c_div * tmp, scaled_rotation.rr, scaled_rotation.ry, scaled_rotation.rp);
    }

    scaled_rotation.camera = get_camera_offset_matrix(c_div);
    real_rotation.camera = get_camera_offset_matrix(1);

    scaled_rotation.rotation = scaled_rotation.camera * scaled_rotation.rotation;
    real_rotation.rotation = real_rotation.camera * real_rotation.rotation;

    bool nanp = is_nan(value) || is_nan(scaled_rotation.rotation) || is_nan(real_rotation.rotation);

    if (!nanp)
    {
        bool can_center = false;

        if (get(f_center))
        {
            using std::fabs;

            for (int i = 0; i < 6; i++)
                if (fabs(newpose(i)) != 0)
                {
                    can_center = true;
                    break;
                }
        }

        if (can_center)
        {
            set(f_center, false);

            if (libs.pFilter)
                libs.pFilter->center();

            if (libs.pTracker->center())
            {
                scaled_rotation.rotation = scaled_rotation.camera.t();
                real_rotation.rotation = real_rotation.camera.t();

                scaled_rotation.rotation = rmat::eye();
                real_rotation.rotation = rmat::eye();
                scaled_rotation.center_roll = rmat::eye();
                scaled_rotation.center_yaw = rmat::eye();
                scaled_rotation.center_pitch = rmat::eye();
            }
            else
            {
                euler::tait_bryan_to_matrices(rmat_to_euler(scaled_rotation.rotation),
                                              scaled_rotation.center_roll,
                                              scaled_rotation.center_pitch,
                                              scaled_rotation.center_yaw);
#if 0
                euler::tait_bryan_to_matrices(rmat_to_euler(real_rotation.rotation),
                                              real_rotation.center_roll,
                                              real_rotation.center_pitch,
                                              real_rotation.center_yaw);
#endif
                real_rotation.rot_center = real_rotation.rotation.t();
                scaled_rotation.rot_center = scaled_rotation.rotation.t();
            }

            t_center = euler_t(&value(TX));
        }
    }

    {
        rmat rotation;

        switch (s.center_method)
        {
        // inertial
        case 0:
        default:
            //scaled_rotation.rotation = scaled_rotation
            rotation = scaled_rotation.rot_center * scaled_rotation.rotation;
            break;
        // camera
        case 1:
            rotation = scaled_rotation.rotation * scaled_rotation.rot_center;
            break;
        // alternative camera
        case 2:
            rmat cy, cp, cr;
            tait_bryan_to_matrices(rmat_to_euler(scaled_rotation.rotation), cr, cp, cy);

            rmat ry = cy * scaled_rotation.center_yaw.t();
            rmat rp = cp * scaled_rotation.center_pitch.t();
            rmat rr = cr * scaled_rotation.center_roll.t();

            // roll yaw pitch
            rotation = rr * ry * rp;
            break;
        }

        const euler_t rot = r2d * c_mult * rmat_to_euler(rotation);
        euler_t pos = euler_t(&value[TX]) - t_center;

        if (s.use_camera_offset_from_centering)
            t_compensate((real_rotation.camera * real_rotation.rot_center).t(), pos, pos, false, false, false);
        else
            t_compensate(real_rotation.camera.t(), pos, pos, false, false, false);

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
        nanp = true;
        logger.write_pose(value); // "filtered"
    }
    else
    {
        Pose tmp(value);

        if (libs.pFilter)
            libs.pFilter->filter(tmp, value);

        logger.write_pose(value); // "filtered"

        // CAVEAT rotation only, due to tcomp
        for (int i = 3; i < 6; i++)
            value(i) = map(value(i), m(i));

        for (int i = 0; i < 6; i++)
            value(i) += m(i).opts.zero;

        if (get(f_zero))
            for (int i = 0; i < 6; i++)
                value(i) = 0;

        if (is_nan(value))
            nanp = true;
    }

    if (s.tcomp_p && !get(f_tcomp_disabled))
    {
        const double tcomp_c[] =
        {
            double(!s.tcomp_disable_src_yaw),
            double(!s.tcomp_disable_src_pitch),
            double(!s.tcomp_disable_src_roll),
        };
        euler_t value_(value(TX), value(TY), value(TZ));
        t_compensate(euler_to_rmat(
                         euler_t(value(Yaw)   * d2r * tcomp_c[0],
                                 value(Pitch) * d2r * tcomp_c[1],
                                 value(Roll)  * d2r * tcomp_c[2])),
                     value_,
                     value_,
                     s.tcomp_disable_tx,
                     s.tcomp_disable_ty,
                     s.tcomp_disable_tz);
        if (is_nan(value_))
            nanp = true;
        for (int i = 0; i < 3; i++)
            value(i) = value_(i);
    }

    // CAVEAT translation only, due to tcomp
    for (int i = 0; i < 3; i++)
        value(i) = map(value(i), m(i));

    for (int i = 0; i < 6; i++)
        if (m(i).opts.invert)
            value(i) = -value(i);

    logger.write_pose(value); // "mapped"

    if (nanp)
    {
        value = last_mapped;

        // for widget last value display
        for (int i = 0; i < 6; i++)
            (void) map(value(i), m(i));
    }

    libs.pProtocol->pose(value);

    last_mapped = value;
    last_raw = raw;

    QMutexLocker foo(&mtx);
    output_pose = value;
    raw_6dof = raw;

    logger.reset_dt();
    logger.next_line();
}

void Tracker::run()
{
#if defined(_WIN32)
    (void) timeBeginPeriod(1);
#endif

    setPriority(QThread::HighPriority);

    {
        static constexpr const char* posechannels[6] = { "TX", "TY", "TZ", "Yaw", "Pitch", "Roll" };
        static constexpr const char* datachannels[5] = { "dt", "raw", "corrected", "filtered", "mapped" };
        logger.write(datachannels[0]);
        char buffer[128];
        for (unsigned j = 1; j < 5; ++j)
        {
            for (unsigned i = 0; i < 6; ++i)
            {
                std::sprintf(buffer, "%s%s", datachannels[j], posechannels[i]);
                logger.write(buffer);
            }
        }
        logger.next_line();
    }

    t.start();
    logger.reset_dt();

    while (!get(f_should_quit))
    {
        Pose tmp;
        libs.pTracker->data(tmp);

        if (get(f_enabled))
            for (int i = 0; i < 6; i++)
                newpose[i] = elide_nan(tmp(i), newpose(i));

        logic();

        static constexpr long const_sleep_us = 4000;

        using std::max;
        using std::min;

        const long elapsed_usecs = t.elapsed_usecs();

        backlog_time += elapsed_usecs - const_sleep_us;

        if (std::fabs(backlog_time) > 10000l * 1000)
        {
            qDebug() << "tracker: backlog interval overflow" << backlog_time;
            backlog_time = 0;
        }

        const unsigned sleep_time = unsigned(std::round(clamp((const_sleep_us - backlog_time)/1000., 0., const_sleep_us*3/1000.)));

        t.start();

        if (sleep_time > 0)
            portable::sleep(sleep_time);
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


void bits::set(bits::flags flag_, bool val_)
{
    unsigned b_(b);
    const unsigned flag = unsigned(flag_);
    const unsigned val = unsigned(!!val_);
    while (!b.compare_exchange_weak(b_,
                                    unsigned((b_ & ~flag) | (flag * val)),
                                    std::memory_order_seq_cst,
                                    std::memory_order_seq_cst))
    { /* empty */ }
}

void bits::negate(bits::flags flag_)
{
    unsigned b_(b);
    const unsigned flag = unsigned(flag_);
    while (!b.compare_exchange_weak(b_,
                                    (b_ & ~flag) | (flag & ~b_),
                                    std::memory_order_seq_cst,
                                    std::memory_order_seq_cst))
    { /* empty */ }
}

bool bits::get(bits::flags flag)
{
    return !!(b & flag);
}

bits::bits() : b(0u)
{
    set(f_center, true);
    set(f_enabled, true);
    set(f_zero, false);
    set(f_tcomp_disabled, false);
    set(f_should_quit, false);
}
