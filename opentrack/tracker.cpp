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
    libs(libs),
    r_b (cv::Matx33d::eye()),
    t_b {0,0,0}
{
}

Tracker::~Tracker()
{
    should_quit = true;
    wait();
}

double Tracker::map(double pos, bool invertp, Mapping& axis)
{
    bool altp = (pos < 0) == !invertp && axis.opts.altp;
    axis.curve.setTrackingActive( !altp );
    axis.curveAlt.setTrackingActive( altp );
    auto& fc = altp ? axis.curveAlt : axis.curve;
    return fc.getValue(pos) + axis.opts.zero;
}

// http://stackoverflow.com/a/18436193
static cv::Vec3d rmat_to_euler(const cv::Matx33d& R)
{
    const double beta  = atan2( -R(2,0), sqrt(R(2,1)*R(2,1) + R(2,2)*R(2,2)) );
    const double alpha = atan2( R(1,0), R(0,0));
    const double gamma = atan2( R(2,1), R(2,2));
    return cv::Vec3d { alpha, beta, gamma };
}

// tait-bryan angles, not euler
static cv::Matx33d euler_to_rmat(const double* input)
{
    static constexpr double pi = 3.141592653;
    const auto H = input[0] * pi / 180;
    const auto P = input[1] * pi / 180;
    const auto B = input[2] * pi / 180;

    const auto c1 = cos(H);
    const auto s1 = sin(H);
    const auto c2 = cos(P);
    const auto s2 = sin(P);
    const auto c3 = cos(B);
    const auto s3 = sin(B);

    double foo[] = {
        // z
        c1 * c2,
        c1 * s2 * s3 - c3 * s1,
        s1 * s3 + c1 * c3 * s2,
        // y
        c2 * s1,
        c1 * c3 + s1 * s2 * s3,
        c3 * s1 * s2 - c1 * s3,
        // x
        -s2,
        c2 * s3,
        c2 * c3
    };

    return cv::Matx33d(foo);
}

void Tracker::t_compensate(const cv::Matx33d& rmat, const double* xyz, double* output, bool rz)
{
    const double xyz_[3] = { xyz[2], xyz[0], -xyz[1] };
    cv::Matx31d tvec(xyz_);
    const cv::Matx31d ret = rmat * tvec;
    if (!rz)
        output[2] = ret(0, 0);
    output[0] = ret(1, 0);
    output[1] = -ret(2, 0);
}

void Tracker::logic()
{
    libs.pTracker->data(newpose);

    if (enabledp)
        for (int i = 0; i < 6; i++)
            final_raw(i) = newpose[i];

    Pose filtered_pose;

    if (libs.pFilter)
        libs.pFilter->filter(final_raw, filtered_pose);
    else
        filtered_pose = final_raw;

    bool inverts[6] = {
        m(0).opts.invert,
        m(1).opts.invert,
        m(2).opts.invert,
        m(3).opts.invert,
        m(4).opts.invert,
        m(5).opts.invert,
    };

    // must invert early as euler_to_rmat's sensitive to sign change
    for (int i = 0; i < 6; i++)
        filtered_pose[i] *= inverts[i] ? -1. : 1.;

    if (centerp)
    {
        centerp = false;
        cv::Matx31d tmp;
        r_b = euler_to_rmat(&filtered_pose[Yaw]);
        for (int i = 0; i < 3; i++)
            t_b[i] = filtered_pose(i);
    }

    Pose raw_centered;

    {
        const cv::Matx33d rmat = euler_to_rmat(&filtered_pose[Yaw]);
        const cv::Matx33d m_ = (r_b * rmat * r_b.t()) * r_b.t();
        const auto euler = rmat_to_euler(m_);
        for (int i = 0; i < 3; i++)
        {
            static constexpr double pi = 3.141592653;
            raw_centered(i) = filtered_pose(i) - t_b[i];
            raw_centered(i+3) = euler(i) * 180./pi;
        }
    }

    Pose mapped_pose_precomp;

    for (int i = 0; i < 6; i++)
        mapped_pose_precomp(i) = map(raw_centered(i), inverts[i], m(i));

    Pose mapped_pose_ = mapped_pose_precomp;

    if (s.tcomp_p)
        t_compensate(euler_to_rmat(&mapped_pose_precomp[Yaw]),
                     mapped_pose_precomp,
                     mapped_pose_,
                     s.tcomp_tz);

    Pose mapped_pose;

    for (int i = 0; i < 6; i++)
    {
        auto& axis = m(i);
        int k = axis.opts.src;
        if (k < 0 || k >= 6)
            mapped_pose(i) = 0;
        else
            mapped_pose(i) = mapped_pose_(i);
    }


    libs.pProtocol->pose(mapped_pose);

    QMutexLocker foo(&mtx);
    output_pose = mapped_pose;
    raw_6dof = final_raw;
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

