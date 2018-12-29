/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt.h"
#include "cv/video-widget.hpp"
#include "compat/camera-names.hpp"
#include "compat/math-imports.hpp"
#include "compat/spinlock.hpp"

#include "pt-api.hpp"

#include <cmath>

#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

namespace pt_module {

Tracker_PT::Tracker_PT(pointer<pt_runtime_traits> const& traits) :
    traits { traits },
    s { traits->get_module_name() },
    point_extractor { traits->make_point_extractor() },
    camera { traits->make_camera() },
    frame { traits->make_frame() },
    preview_frame { traits->make_preview(preview_width, preview_height) }
{
    cv::setBreakOnError(true);
    cv::setNumThreads(1);

    connect(s.b.get(), SIGNAL(saving()), this, SLOT(maybe_reopen_camera()), Qt::DirectConnection);
    connect(&s.fov, SIGNAL(valueChanged(int)), this, SLOT(set_fov(int)), Qt::DirectConnection);
    set_fov(s.fov);
}

Tracker_PT::~Tracker_PT()
{
    requestInterruption();
    wait();

    QMutexLocker l(&camera_mtx);
    camera->stop();
}

void Tracker_PT::run()
{
    maybe_reopen_camera();

    while(!isInterruptionRequested())
    {
        pt_camera_info info;
        bool new_frame = false;

        {
            QMutexLocker l(&camera_mtx);

            if (camera)
                std::tie(new_frame, info) = camera->get_frame(*frame);
        }

        if (new_frame)
        {
            spinlock_guard l(center_flag);

            *preview_frame = *frame;

            point_extractor->extract_points(*frame, *preview_frame, points);
            point_count = points.size();

            const double fx = pt_camera_info::get_focal_length(info.fov, info.res_x, info.res_y);

            const bool success = points.size() >= PointModel::N_POINTS;

            if (success)
            {
                point_tracker.track(points,
                                    PointModel(s),
                                    info,
                                    s.dynamic_pose ? s.init_phase_timeout : 0);
                ever_success = true;
            }

            {
                Affine X_CM;
                {
                    QMutexLocker l(&data_mtx);
                    X_CM = point_tracker.pose();
                }

                // just copy pasted these lines from below
                Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z));
                Affine X_GH = X_CM * X_MH;
                vec3 p = X_GH.t; // head (center?) position in global space

                preview_frame->draw_head_center((p[0] * fx) / p[2], (p[1] * fx) / p[2]);
            }

            video_widget->update_image(preview_frame->get_bitmap());

            {
                int w = -1, h = -1;
                video_widget->get_preview_size(w, h);
                if (w != preview_width || h != preview_height)
                {
                    preview_width = w; preview_height = h;
                    preview_frame = traits->make_preview(w, h);
                }
            }
        }
    }
}

bool Tracker_PT::maybe_reopen_camera()
{
    QMutexLocker l(&camera_mtx);

    return camera->start(camera_name_to_index(s.camera_name),
                         s.cam_fps, s.cam_res_x, s.cam_res_y);
}

void Tracker_PT::set_fov(int value)
{
    QMutexLocker l(&camera_mtx);
    camera->set_fov(value);
}

module_status Tracker_PT::start_tracker(QFrame* video_frame)
{
    //video_frame->setAttribute(Qt::WA_NativeWindow);

    video_widget = std::make_unique<cv_video_widget>(video_frame);
    layout = std::make_unique<QHBoxLayout>(video_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(video_widget.get());
    video_frame->setLayout(layout.get());
    //video_widget->resize(video_frame->width(), video_frame->height());
    video_frame->show();

    start(QThread::HighPriority);

    return {};
}

void Tracker_PT::data(double *data)
{
    if (ever_success)
    {
        Affine X_CM = pose();

        Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z));
        Affine X_GH = X_CM * X_MH;

        // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
        // -z -> x, y -> z, x -> -y
        mat33 R_EG(0, 0,-1,
                   -1, 0, 0,
                   0, 1, 0);
        mat33 R = R_EG *  X_GH.R * R_EG.t();

        // get translation(s)
        const vec3& t = X_GH.t;

        // extract rotation angles
        {
            f alpha, beta, gamma;
            beta  = atan2( -R(2,0), sqrt(R(2,1)*R(2,1) + R(2,2)*R(2,2)) );
            alpha = atan2( R(1,0), R(0,0));
            gamma = atan2( R(2,1), R(2,2));

            data[Yaw] = rad2deg * alpha;
            data[Pitch] = -rad2deg * beta;
            data[Roll] = rad2deg * gamma;
        }

        // convert to cm
        data[TX] = t[0] / 10;
        data[TY] = t[1] / 10;
        data[TZ] = t[2] / 10;
    }
}

bool Tracker_PT::center()
{
    spinlock_guard l(center_flag);

    point_tracker.reset_state();
    return false;
}

Affine Tracker_PT::pose()
{
    QMutexLocker l(&data_mtx);

    return point_tracker.pose();
}

int Tracker_PT::get_n_points()
{
    return int(point_count);
}

bool Tracker_PT::get_cam_info(pt_camera_info* info)
{
    QMutexLocker lock(&camera_mtx);
    bool ret;

    std::tie(ret, *info) = camera->get_info();
    return ret;
}

} // ns pt_module
