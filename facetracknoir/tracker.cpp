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
#include <opencv2/core/core.hpp>
#include <cmath>
#include <algorithm>

#if defined(_WIN32)
#   include <windows.h>
#endif

Tracker::Tracker(main_settings& s, Mappings &m) :
    s(s),
    m(m),
    centerp(false),
    enabledp(true),
    should_quit(false)
{
}

Tracker::~Tracker()
{
    should_quit = true;
    wait();
}

void Tracker::get_curve(double pos, double& out, Mapping& axis) {
    bool altp = (pos < 0) && axis.opts.altp;
    axis.curve.setTrackingActive( !altp );
    axis.curveAlt.setTrackingActive( altp );
    auto& fc = altp ? axis.curveAlt : axis.curve;
    out = (axis.opts.invert ? -1 : 1) * fc.getValue(pos);

    out += axis.opts.zero;
}

void Tracker::t_compensate(const double* input, double* output, bool rz)
{
    static constexpr double pi = 3.141592653;
    const auto H = input[Yaw] * pi / -180;
    const auto P = input[Pitch] * pi / -180;
    const auto B = input[Roll] * pi / 180;

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

    const cv::Matx33d rmat(foo);
    const cv::Vec3d tvec(input);
    const cv::Vec3d ret = rmat * tvec;

    const int max = !rz ? 3 : 2;

    for (int i = 0; i < max; i++)
        output[i] = ret(i);
}

void Tracker::run() {
    Pose pose_offset, unstopped_pose;

    double newpose[6] = {0};
    const int sleep_ms = 3;

#if defined(_WIN32)
    (void) timeBeginPeriod(1);
#endif

    while (!should_quit)
    {
        t.start();

        Libraries->pTracker->GetHeadPoseData(newpose);

        Pose final_raw, filtered;

        for (int i = 0; i < 6; i++)
        {
            auto& axis = m(i);
            int k = axis.opts.src;
            if (k < 0 || k >= 6)
                continue;
            // not really raw, after axis remap -sh
            final_raw(i) = newpose[k];
        }

        {
            if (enabledp)
                unstopped_pose = final_raw;

            if (Libraries->pFilter)
                Libraries->pFilter->FilterHeadPoseData(unstopped_pose, filtered);
            else
                filtered = unstopped_pose;
            
            if (centerp)  {
                centerp = false;
                pose_offset = filtered;
            }

            filtered = filtered & pose_offset;

            for (int i = 0; i < 6; i++)
                get_curve(filtered(i), filtered(i), m(i));
        }

        if (s.tcomp_p)
            t_compensate(filtered, filtered, s.tcomp_tz);

        Libraries->pProtocol->sendHeadposeToGame(filtered);

        {
            QMutexLocker foo(&mtx);
            output_pose = filtered;
            raw_6dof = final_raw;
        }

        const long q = 1000L * std::max(0L, sleep_ms - t.elapsed_ms());
        usleep(q);
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

