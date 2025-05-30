/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#undef NDEBUG
#include "ftnoir_tracker_pt.h"
#include "pt-api.hpp"
#include "cv/init.hpp"
#include "video/video-widget.hpp"
#include "compat/math-imports.hpp"
#include "compat/check-visible.hpp"
#include "compat/thread-name.hpp"
#include "compat/qt-dpi.hpp"

#include <cassert>
#include <QHBoxLayout>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>

using namespace options;

namespace pt_impl {

Tracker_PT::Tracker_PT(pointer<pt_runtime_traits> const& traits) :
    traits { traits },
    s { traits->get_module_name() },
    point_extractor { traits->make_point_extractor() },
    frame { traits->make_frame() }
{
    opencv_init();

    connect(&*s.b, &bundle_::saving, this, [this]{ reopen_camera_flag = true; }, Qt::DirectConnection);
}

Tracker_PT::~Tracker_PT()
{
    requestInterruption();
    wait();

    if (camera)
        camera->stop();
}

bool Tracker_PT::check_camera()
{
    if (reopen_camera_flag)
    {
        reopen_camera_flag = false;

        camera = nullptr;
        camera = traits->make_camera();
        if (!camera || !camera->start(s))
            return false;
    }
    assert(camera);
    if (progn(bool x = true; return open_camera_dialog_flag.compare_exchange_strong(x, false);))
        run_in_thread_sync(qApp->thread(), [this] { camera->show_camera_settings(); });
    return true;
}

void Tracker_PT::run()
{
    portable::set_curthread_name("tracker/pt");

    while(!isInterruptionRequested())
    {
        if (!check_camera())
            break;

        pt_camera_info info;
        bool new_frame = false;

        {
            camera->set_fov(s.fov);
            std::tie(new_frame, info) = camera->get_frame(*frame);
        }

        if (new_frame)
        {
            const bool preview_visible = check_is_visible();

            if (preview_visible && !widget->fresh())
                preview_frame->set_last_frame(*frame);

            point_extractor->extract_points(*frame, *preview_frame, preview_visible && !widget->fresh(), points);
            point_count.store((unsigned)points.size(), std::memory_order_relaxed);

            const bool success = points.size() >= PointModel::N_POINTS;

            Affine X_CM;

            {
                QMutexLocker l(&center_lock);

                if (success)
                {
                    int dynamic_pose_ms = s.dynamic_pose ? s.init_phase_timeout : 0;

                    point_tracker.track(points, PointModel(s), info, dynamic_pose_ms, filter, camera->deadzone_amount());
                    ever_success.store(true, std::memory_order_relaxed);
                }

                QMutexLocker l2(&data_lock);
                X_CM = point_tracker.pose();
            }

            if (preview_visible && !widget->fresh())
            {
                const f fx = pt_camera_info::get_focal_length(info.fov, info.res_x, info.res_y);
                Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z));
                Affine X_GH = X_CM * X_MH;
                vec3 p = X_GH.t; // head (center?) position in global space

                if (p[2] > f(.1))
                    preview_frame->draw_head_center((p[0] * fx) / p[2], (p[1] * fx) / p[2]);

                widget->update_image(preview_frame->get_bitmap());
            }
        }
    }
}

module_status Tracker_PT::start_tracker(QFrame* video_frame)
{
    {
        auto camera = traits->make_camera();
        if (!camera || !camera->start(s))
            return error(tr("Failed to open camera '%1'").arg(s.camera_name));
    }

    widget = std::make_unique<video_widget>(video_frame);
    layout = std::make_unique<QHBoxLayout>(video_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&*widget);
    video_frame->setLayout(&*layout);
    //video_widget->resize(video_frame->width(), video_frame->height());
    video_frame->show();

    double dpi = screen_dpi(video_frame);
    preview_frame = traits->make_preview(iround(preview_width * dpi),
                                         iround(preview_height * dpi));

    start(QThread::HighPriority);

    return {};
}

void Tracker_PT::data(double *data)
{
    if (ever_success.load(std::memory_order_relaxed))
    {
        Affine X_CM;
        {
            QMutexLocker l(&data_lock);
            X_CM = point_tracker.pose();
        }

        Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z));
        Affine X_GH(X_CM * X_MH);

        // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
        // -z -> x, y -> z, x -> -y
        mat33 R_EG(0, 0,-1,
                   -1, 0, 0,
                   0, 1, 0);
        mat33 R(R_EG *  X_GH.R * R_EG.t());

        // get translation(s)
        const vec3& t = X_GH.t;

        // extract rotation angles
        auto r00 = (double)R(0, 0);
        auto r10 = (double)R(1,0), r20 = (double)R(2,0);
        auto r21 = (double)R(2,1), r22 = (double)R(2,2);

        double beta  = atan2(-r20, sqrt(r21*r21 + r22*r22));
        double alpha = atan2(r10, r00);
        double gamma = atan2(r21, r22);

        constexpr double rad2deg = 180/M_PI;

        data[Yaw]   = rad2deg * alpha;
        data[Pitch] = -rad2deg * beta;
        data[Roll]  = rad2deg * gamma;

        // convert to cm
        data[TX] = (double)t[0] / 10;
        data[TY] = (double)t[1] / 10;
        data[TZ] = (double)t[2] / 10;
    }
}

bool Tracker_PT::center()
{
    QMutexLocker l(&center_lock);

    point_tracker.reset_state();
    return false;
}

int Tracker_PT::get_n_points()
{
    return (int)point_count.load(std::memory_order_relaxed);
}

bool Tracker_PT::get_cam_info(pt_camera_info& info)
{
    bool ret = false;

    if (camera)
        std::tie(ret, info) = camera->get_info();
    return ret;
}

Affine Tracker_PT::pose() const
{
    QMutexLocker l(&data_lock);
    return point_tracker.pose();
}

} // ns pt_impl
