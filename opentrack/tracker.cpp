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

#include <opencv2/core/core.hpp>

#include "./tracker.h"
#include <cmath>
#include <algorithm>

#if defined(_WIN32)
#   include <windows.h>
#endif

Tracker::Tracker(main_settings& s, Mappings &m, SelectedLibraries &libs) :
    s(s),
    m(m),
    centerp(false),
    enabledp(true),
    should_quit(false),
    libs(libs)
{
}

Tracker::~Tracker()
{
    should_quit = true;
    wait();
}

double Tracker::map(double pos, Mapping& axis) {
    bool altp = (pos < 0) && axis.opts.altp;
    axis.curve.setTrackingActive( !altp );
    axis.curveAlt.setTrackingActive( altp );
    auto& fc = altp ? axis.curveAlt : axis.curve;
    double invert = axis.opts.invert ? -1 : 1;
    return invert * (fc.getValue(pos) + axis.opts.zero);
}

static cv::Matx33d euler_to_rmat(const double* input)
{
    static constexpr double pi = 3.141592653;
    const auto H = input[0] * pi / -180;
    const auto P = input[1] * pi / -180;
    const auto B = input[2] * pi / 180;

    const auto cosH = cos(H);
    const auto sinH = sin(H);
    const auto cosP = cos(P);
    const auto sinP = sin(P);
    const auto cosB = cos(B);
    const auto sinB = sin(B);

    double foo[] = {
        cosH * cosB - sinH * sinP * sinB,
        - sinB * cosP,
        sinH * cosB + cosH * sinP * sinB,
        cosH * sinB + sinH * sinP * cosB,
        cosB * cosP,
        sinB * sinH - cosH * sinP * cosB,
        - sinH * cosP,
        - sinP,
        cosH * cosP,
    };

    return cv::Matx33d(foo);    
}

void Tracker::t_compensate(const double* input, double* output, bool rz)
{
    const cv::Matx33d rmat = euler_to_rmat(&input[Yaw]);
    const cv::Vec3d tvec(input);
    const cv::Vec3d ret = rmat * tvec;

    const int max = !rz ? 3 : 2;

    for (int i = 0; i < max; i++)
        output[i] = ret(i);
}

void Tracker::logic()
{
    libs.pTracker->data(newpose);
    
    Pose final_raw_;

    if (enabledp)
    {
        for (int i = 0; i < 6; i++)
        {
            auto& axis = m(i);
            int k = axis.opts.src;
            if (k < 0 || k >= 6)
            {
                final_raw_(i) = 0;
                continue;
            }
            // not really raw, after axis remap -sh
            final_raw_(i) = newpose[k];
        }
        final_raw = final_raw_;
    }
    
    Pose filtered_pose;
    
    if (libs.pFilter)
        libs.pFilter->filter(final_raw, filtered_pose);
    else
        filtered_pose = final_raw;
    
    if (centerp)
    {
        centerp = false;
        raw_center = filtered_pose;
    }
    
    Pose raw_centered = filtered_pose & raw_center;
    
    Pose mapped_pose_precomp;
    
    for (int i = 0; i < 6; i++)
        mapped_pose_precomp(i) = map(raw_centered(i), m(i));
    
    Pose mapped_pose;
    
    mapped_pose = mapped_pose_precomp;
    if (s.tcomp_p)
        t_compensate(mapped_pose_precomp, mapped_pose, s.tcomp_tz);

    libs.pProtocol->pose(mapped_pose);

    {
        QMutexLocker foo(&mtx);
        output_pose = mapped_pose;
        raw_6dof = final_raw;
    }
}

void Tracker::run() {
    const int sleep_ms = 3;

#if defined(_WIN32)
    (void) timeBeginPeriod(1);
#endif

    while (!should_quit)
    {
        t.start();

        logic();

        double q = sleep_ms * 1000L;
        q -= t.elapsed();
        q = std::max(0., q);
        usleep((long)q);
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

