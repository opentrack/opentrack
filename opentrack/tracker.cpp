/* Copyright (c) 2012-2013 Stanislaw Halik <sthalik@misaki.pl>
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
#include "opentrack/thread.hpp"
#include <cmath>
#include <algorithm>

#if defined(_WIN32)
#   include <windows.h>
#endif

Tracker::Tracker(main_settings& s, Mappings &m, SelectedLibraries &libs) :
    s(s),
    m(m),
    centerp(s.center_at_startup),
    enabledp(true),
    zero_(false),
    should_quit(false),
    libs(libs),
    r_b(dmat<3,3>::eye()),
    t_b {0,0,0}
{
}

Tracker::~Tracker()
{
    should_quit = true;
    wait();
}

double Tracker::map(double pos, Mapping& axis)
{
    bool altp = (pos < 0) && axis.opts.altp;
    axis.curve.setTrackingActive( !altp );
    axis.curveAlt.setTrackingActive( altp );
    auto& fc = altp ? axis.curveAlt : axis.curve;
    return fc.getValue(pos);
}

void Tracker::t_compensate(const rmat& rmat, const double* xyz, double* output, bool rz)
{
    // TY is really yaw axis. need swapping accordingly.
    dmat<3, 1> tvec({ xyz[2], -xyz[0], -xyz[1] });
    const dmat<3, 1> ret = rmat * tvec;
    if (!rz)
        output[2] = ret(0, 0);
    else
        output[2] = xyz[2];
    output[1] = -ret(2, 0);
    output[0] = -ret(1, 0);
}

void Tracker::logic()
{
    bool inverts[6] = {
        m(0).opts.invert,
        m(1).opts.invert,
        m(2).opts.invert,
        m(3).opts.invert,
        m(4).opts.invert,
        m(5).opts.invert,
    };
    
    static constexpr double pi = 3.141592653;
    static constexpr double r2d = 180. / pi;
    
    Pose value, raw;
    
    if (!zero_)
        for (int i = 0; i < 6; i++)
        {
            value(i) = newpose[i];
            raw(i) = newpose[i];
        }
    else
    {
        auto mat = rmat::rmat_to_euler(r_b);
        
        for (int i = 0; i < 3; i++)
        {
            raw(i+3) = value(i+3) = mat(i, 0) * r2d;
            raw(i) = value(i) = t_b[i];
        }
    }

    const double off[] = {
        (double)-s.camera_yaw,
        (double)-s.camera_pitch,
        (double)s.camera_roll
    };
    const rmat cam = rmat::euler_to_rmat(off);
    rmat r = rmat::euler_to_rmat(&value[Yaw]);
    dmat<3, 1> t { value(0), value(1), value(2) };
    
    r = cam * r;
    t = cam * t;
    
    bool can_center = false;
    
    if (centerp)
    {
        for (int i = 0; i < 6; i++)
            if (fabs(newpose[i]) != 0)
        {
            can_center = true;
            break;
        }
    }
    
    if (can_center)
    {
        centerp = false;
        for (int i = 0; i < 3; i++)
            t_b[i] = t(i, 0);
        r_b = r;
    }
    
    {
        double tmp[3] = { t(0, 0) - t_b[0], t(1, 0) - t_b[1], t(2, 0) - t_b[2] };
        t_compensate(cam, tmp, tmp, false);
        const rmat m_ = r_b.t() * r;
        const dmat<3, 1> euler = rmat::rmat_to_euler(m_);
        for (int i = 0; i < 3; i++)
        {
            value(i) = tmp[i];
            value(i+3) = euler(i, 0) * r2d;
        }
    }
    
    {
        Pose tmp = value;
        
        if (libs.pFilter)
            libs.pFilter->filter(tmp, value);
    }
    
    for (int i = 0; i < 6; i++)
        value(i) = map(value(i), m(i));
    
    if (s.tcomp_p)
        t_compensate(rmat::euler_to_rmat(&value[Yaw]),
                     value,
                     value,
                     s.tcomp_tz);
    
    for (int i = 0; i < 6; i++)
        value[i] *= inverts[i] ? -1. : 1.;

    Pose output_pose_;

    for (int i = 0; i < 6; i++)
    {
        auto& axis = m(i);
        int k = axis.opts.src;
        if (k < 0 || k >= 6)
            output_pose_(i) = 0;
        else
            output_pose_(i) = value(k);
    }


    libs.pProtocol->pose(output_pose_);

    QMutexLocker foo(&mtx);
    output_pose = output_pose_;
    raw_6dof = raw;
}

void Tracker::run() {
    const int sleep_ms = 3;
    
    Affinity thr(CORE_IPC);

#if defined(_WIN32)
    (void) timeBeginPeriod(1);
#endif

    while (!should_quit)
    {
        t.start();
        
        double tmp[6] {0,0,0, 0,0,0};
        libs.pTracker->data(tmp);
        
        if (enabledp)
            for (int i = 0; i < 6; i++)
                newpose[i] = tmp[i];
        
        logic();

        long q = sleep_ms * 1000L - t.elapsed()/1000L;
        usleep(std::max(1L, q));
    }

    {
        // do one last pass with origin pose
        for (int i = 0; i < 6; i++)
            newpose[i] = 0;
        logic();
        // filter may inhibit exact origin
        Pose p;
        libs.pProtocol->pose(p);
    }

#if defined(_WIN32)
    (void) timeEndPeriod(1);
#endif

    for (int i = 0; i < 6; i++)
    {
        m(i).curve.setTrackingActive(false);
        m(i).curveAlt.setTrackingActive(false);
    }
}

void Tracker::get_raw_and_mapped_poses(double* mapped, double* raw) const {
    QMutexLocker foo(&const_cast<Tracker&>(*this).mtx);
    for (int i = 0; i < 6; i++)
    {
        raw[i] = raw_6dof(i);
        mapped[i] = output_pose(i);
    }
}

