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
#include "facetracknoir.h"
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

static void t_compensate(double* input, double* output, bool rz)
{
    const auto H = input[Yaw] * M_PI / -180;
    const auto P = input[Pitch] * M_PI / -180;
    const auto B = input[Roll] * M_PI / 180;

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

    cv::Mat rmat(3, 3, CV_64F, foo);
    const cv::Mat tvec(3, 1, CV_64F, input);
    cv::Mat ret = rmat * tvec;

    const int max = !rz ? 3 : 2;

    for (int i = 0; i < max; i++)
        output[i] = ret.at<double>(i);
}

void Tracker::run() {
    T6DOF pose_offset, unstopped_pose;

    double newpose[6] = {0};
    int sleep_ms = 15;

    if (Libraries->pTracker)
        sleep_ms = std::min(sleep_ms, 1000 / Libraries->pTracker->preferredHz());

    qDebug() << "tracker Hz:" << 1000 / sleep_ms;

#if defined(_WIN32)
    (void) timeBeginPeriod(1);
#endif

    for (;;)
    {
        t.start();

        if (should_quit)
            break;

        if (Libraries->pTracker) {
            Libraries->pTracker->GetHeadPoseData(newpose);
        }

        {
            QMutexLocker foo(&mtx);

            for (int i = 0; i < 6; i++)
            {
                auto& axis = m(i);
                int k = axis.opts.src;
                if (k < 0 || k >= 6)
                    continue;
                // not really raw, after axis remap -sh
                raw_6dof(i) = newpose[k];
            }

            if (centerp)  {
                centerp = false;
                pose_offset = raw_6dof;
            }

            {
                if (enabledp)
                    unstopped_pose = raw_6dof;

                {

                    if (Libraries->pFilter)
                        Libraries->pFilter->FilterHeadPoseData(unstopped_pose, output_pose);
                    else
                        output_pose = unstopped_pose;

                    output_pose = output_pose - pose_offset;
                }

                for (int i = 0; i < 6; i++)
                    get_curve(output_pose(i), output_pose(i), m(i));
            }

            if (s.tcomp_p)
                t_compensate(output_pose, output_pose, s.tcomp_tz);

            if (Libraries->pProtocol) {
                Libraries->pProtocol->sendHeadposeToGame(output_pose);
            }
        }

        const long q = std::max(0L, sleep_ms * 1000L - std::max(0L, t.elapsed()));

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

