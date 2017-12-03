/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt.h"
#include "compat/camera-names.hpp"
#include "compat/math-imports.hpp"
#include <QHBoxLayout>
#include <cmath>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <functional>

Tracker_PT::Tracker_PT() :
      point_count(0),
      commands(0),
      ever_success(false)
{
    cv::setBreakOnError(true);

    connect(s.b.get(), SIGNAL(saving()), this, SLOT(maybe_reopen_camera()), Qt::DirectConnection);
    connect(&s.fov, SIGNAL(valueChanged(int)), this, SLOT(set_fov(int)), Qt::DirectConnection);
    set_fov(s.fov);
}

Tracker_PT::~Tracker_PT()
{
    set_command(ABORT);
    wait();

    QMutexLocker l(&camera_mtx);
    camera.stop();
}

void Tracker_PT::set_command(Command command)
{
    //QMutexLocker lock(&mutex);
    commands |= command;
}

void Tracker_PT::reset_command(Command command)
{
    //QMutexLocker lock(&mutex);
    commands &= ~command;
}

void Tracker_PT::run()
{
    cv::setNumThreads(1);

#ifdef PT_PERF_LOG
    QFile log_file(OPENTRACK_BASE_PATH + "/PointTrackerPerformance.txt");
    if (!log_file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream log_stream(&log_file);
#endif

    while((commands & ABORT) == 0)
    {
        CamInfo cam_info;
        bool new_frame = false;

        {
            QMutexLocker l(&camera_mtx);

            if (camera)
                std::tie(new_frame, cam_info) = camera.get_frame(frame);
        }

        if (new_frame)
        {
            cv::resize(frame, preview_frame,
                       cv::Size(preview_size.width(), preview_size.height()),
                       0, 0, cv::INTER_NEAREST);

            point_extractor.extract_points(frame, preview_frame, points);
            point_count = points.size();

            const double fx = cam_info.get_focal_length();

            const bool success = points.size() >= PointModel::N_POINTS;

            if (success)
            {
                point_tracker.track(points,
                                    PointModel(s),
                                    cam_info,
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
                vec2 p_((p[0] * fx) / p[2], (p[1] * fx) / p[2]);  // projected to screen

                constexpr int len = 9;

                cv::Point p2(iround(p_[0] * preview_frame.cols + preview_frame.cols/2),
                             iround(-p_[1] * preview_frame.cols + preview_frame.rows/2));
                static const cv::Scalar color(0, 255, 255);
                cv::line(preview_frame,
                         cv::Point(p2.x - len, p2.y),
                         cv::Point(p2.x + len, p2.y),
                         color,
                         1);
                cv::line(preview_frame,
                         cv::Point(p2.x, p2.y - len),
                         cv::Point(p2.x, p2.y + len),
                         color,
                         1);
            }

            video_widget->update_image(preview_frame);
        }
    }
    qDebug() << "pt: thread stopped";
}

void Tracker_PT::maybe_reopen_camera()
{
    QMutexLocker l(&camera_mtx);

    Camera::open_status status = camera.start(camera_name_to_index(s.camera_name), s.cam_fps, s.cam_res_x, s.cam_res_y);

    switch (status)
    {
    case Camera::open_error:
        break;
    case Camera::open_ok_change:
        frame = cv::Mat();
        break;
    case Camera::open_ok_no_change:
        break;
    }
}

void Tracker_PT::set_fov(int value)
{
    QMutexLocker l(&camera_mtx);
    camera.set_fov(value);
}

module_status Tracker_PT::start_tracker(QFrame* video_frame)
{
    //video_frame->setAttribute(Qt::WA_NativeWindow);
    preview_size = video_frame->size();

    preview_frame = cv::Mat(video_frame->height(), video_frame->width(), CV_8UC3);
    preview_frame.setTo(cv::Scalar(0, 0, 0));

    video_widget = std::make_unique<cv_video_widget>(video_frame);
    layout = std::make_unique<QHBoxLayout>(video_frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(video_widget.get());
    video_frame->setLayout(layout.get());
    //video_widget->resize(video_frame->width(), video_frame->height());
    video_frame->show();

    maybe_reopen_camera();

    start(QThread::HighPriority);

    return status_ok();
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

#if 0
            if (t[2] > 1e-4)
            {
                alpha += copysign(atan(t[0] / t[2]), t[0]);
                // pitch is skewed anyway due to only one focal length value
                //beta -= copysign(atan(t[1] / t[2]), t[1]);
            }
#endif

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

Affine Tracker_PT::pose()
{
    QMutexLocker l(&data_mtx);

    return point_tracker.pose();
}

int Tracker_PT::get_n_points()
{
    return int(point_count);
}

bool Tracker_PT::get_cam_info(CamInfo* info)
{
    QMutexLocker lock(&camera_mtx);
    bool ret;

    std::tie(ret, *info) = camera.get_info();
    return ret;
}

#include "ftnoir_tracker_pt_dialog.h"
OPENTRACK_DECLARE_TRACKER(Tracker_PT, TrackerDialog_PT, PT_metadata)

