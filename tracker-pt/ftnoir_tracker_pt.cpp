/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt.h"
#include "compat/camera-names.hpp"
#include <QHBoxLayout>
#include <cmath>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <functional>

//#define PT_PERF_LOG	//log performance

//-----------------------------------------------------------------------------
Tracker_PT::Tracker_PT() :
      video_widget(nullptr),
      video_frame(nullptr),
      point_count(0),
      commands(0),
      ever_success(false)
{
    connect(s.b.get(), SIGNAL(saving()), this, SLOT(apply_settings()));
}

Tracker_PT::~Tracker_PT()
{
    set_command(ABORT);
    wait();
    if (video_widget)
        delete video_widget;
    video_widget = NULL;
    if (video_frame)
    {
        if (video_frame->layout()) delete video_frame->layout();
    }
    // fast start/stop causes breakage
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

bool Tracker_PT::get_focal_length(f& ret)
{
    QMutexLocker l(&camera_mtx);
    CamInfo info;
    const bool res = camera.get_info(info);
    if (res)
    {
        using std::tan;
        using std::atan;
        using std::sqrt;

        const double w = info.res_x, h = info.res_y;
        const double diag = sqrt(w/h*w/h + h/w*h/w);
        const double diag_fov = static_cast<int>(s.fov) * M_PI / 180.;
        const double fov = 2.*atan(tan(diag_fov/2.)/diag);
        ret = .5 / tan(.5 * fov);
        return true;
    }
    return false;
}

void Tracker_PT::run()
{
    cv::setNumThreads(0);

#ifdef PT_PERF_LOG
    QFile log_file(QCoreApplication::applicationDirPath() + "/PointTrackerPerformance.txt");
    if (!log_file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
    QTextStream log_stream(&log_file);
#endif

    apply_settings();
    cv::Mat frame_;

    while((commands & ABORT) == 0)
    {
        const double dt = time.elapsed_seconds();
        time.start();
        bool new_frame;

        {
            QMutexLocker l(&camera_mtx);
            new_frame = camera.get_frame(dt, &frame);
            if (frame.rows != frame_.rows || frame.cols != frame_.cols)
                frame_ = cv::Mat(frame.rows, frame.cols, CV_8UC3);
            frame.copyTo(frame_);
        }

        if (new_frame && !frame_.empty())
        {
            CamInfo cam_info;

            if (!camera.get_info(cam_info))
                continue;

            point_extractor.extract_points(frame_, points);
            point_count = points.size();

            f fx;

            if (!get_focal_length(fx))
                continue;

            const bool success = points.size() >= PointModel::N_POINTS;

            if (success)
            {
                point_tracker.track(points,
                                    PointModel(s),
                                    fx,
                                    s.dynamic_pose,
                                    s.init_phase_timeout,
                                    cam_info.res_x,
                                    cam_info.res_y);
                ever_success = true;
            }

            auto fun = [&](const vec2& p, const cv::Scalar& color)
            {
                using std::round;
                cv::Point p2(int(round(p[0] * frame_.cols + frame_.cols/2)),
                             int(round(-p[1] * frame_.cols + frame_.rows/2)));
                cv::line(frame_,
                         cv::Point(p2.x - 20, p2.y),
                         cv::Point(p2.x + 20, p2.y),
                         color,
                         2);
                cv::line(frame_,
                         cv::Point(p2.x, p2.y - 20),
                         cv::Point(p2.x, p2.y + 20),
                         color,
                         2);
            };

            for (unsigned i = 0; i < points.size(); i++)
            {
                fun(points[i], cv::Scalar(0, 255, 0));
            }

            {
                Affine X_CM;
                {
                    QMutexLocker l(&data_mtx);
                    X_CM = point_tracker.pose();
                }

                Affine X_MH(mat33::eye(), vec3(s.t_MH_x, s.t_MH_y, s.t_MH_z)); // just copy pasted these lines from below
                Affine X_GH = X_CM * X_MH;
                vec3 p = X_GH.t; // head (center?) position in global space
                vec2 p_(p[0] / p[2] * fx, p[1] / p[2] * fx);  // projected to screen
                fun(p_, cv::Scalar(0, 0, 255));
            }

            video_widget->update_image(frame_);
        }
    }
    qDebug() << "pt: Thread stopping";
}

void Tracker_PT::apply_settings()
{
    qDebug() << "pt: applying settings";

    QMutexLocker l(&camera_mtx);

    CamInfo info = camera.get_desired();
    const QString name = camera.get_desired_name();

    if (s.cam_fps != info.fps ||
        s.cam_res_x != info.res_x ||
        s.cam_res_y != info.res_y ||
        s.camera_name != name)
    {
        qDebug() << "pt: starting camera";
        camera.stop();
        camera.set_device(s.camera_name);
        camera.set_res(s.cam_res_x, s.cam_res_y);
        camera.set_fps(s.cam_fps);
        frame = cv::Mat();
        camera.start();
    }

    qDebug() << "pt: done applying settings";
}

void Tracker_PT::start_tracker(QFrame *parent_window)
{
    video_frame = parent_window;
    video_frame->setAttribute(Qt::WA_NativeWindow);
    video_frame->show();
    video_widget = new cv_video_widget(video_frame);
    QHBoxLayout* video_layout = new QHBoxLayout(parent_window);
    video_layout->setContentsMargins(0, 0, 0, 0);
    video_layout->addWidget(video_widget);
    video_frame->setLayout(video_layout);
    video_widget->resize(video_frame->width(), video_frame->height());
    start();
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

        using std::atan2;
        using std::sqrt;
        using std::atan;
        using std::fabs;
        using std::copysign;

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

    return camera.get_info(*info);
}

#include "ftnoir_tracker_pt_dialog.h"
OPENTRACK_DECLARE_TRACKER(Tracker_PT, TrackerDialog_PT, PT_metadata)

