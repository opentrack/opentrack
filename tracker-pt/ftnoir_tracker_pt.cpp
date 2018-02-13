/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt.h"
#include <QHBoxLayout>
#include <cmath>
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include "opentrack-compat/camera-names.hpp"
#include "opentrack-compat/sleep.hpp"
#include <functional>

//#define PT_PERF_LOG	//log performance

//-----------------------------------------------------------------------------
Tracker_PT::Tracker_PT() :
    video_widget(NULL),
    video_frame(NULL),
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
    portable::sleep(1000);
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

bool Tracker_PT::get_focal_length(double& ret)
{
    static constexpr const double fov_ = 75;

    QMutexLocker l(&camera_mtx);
    CamInfo info;
    const bool res = camera.get_info(info);
    if (res)
    {
        using std::tan;
        using std::atan;
        using std::sqrt;

        const int w = info.res_x, h = info.res_y;
        const double diag = sqrt(1. + h/(double)w * h/(double)w);
        const double diag_fov = fov_ * M_PI / 180.;
        const double fov = 2.*atan(tan(diag_fov/2.0)/diag);
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
            QMutexLocker l(&points_mtx);

            point_extractor.extract_points(frame_, points);

            double fx;
            if (!get_focal_length(fx))
                continue;

            const bool success = points.size() >= PointModel::N_POINTS;

            if (success)
            {
                const CamInfo info = camera.get_desired();

                point_tracker.track(points,
                                    PointModel(s),
                                    fx,
                                    s.model_used == PointModel::Cap,
                                    500,
                                    info.res_x,
                                    info.res_y);
                ever_success = true;
            }

            Affine X_CM = pose();

            auto fun = [&](const cv::Vec2d& p, const cv::Scalar& color)
            {
                using std::round;

                cv::Point p2(round(p[0] * frame_.cols + frame_.cols/2),
                             round(-p[1] * frame_.cols + frame_.rows/2));

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
                Affine X_MH(cv::Matx33d::eye(), get_model_offset()); // just copy pasted these lines from below
                Affine X_GH = X_CM * X_MH;
                cv::Vec3d p = X_GH.t; // head (center?) position in global space
                cv::Vec2d p_(p[0] / p[2] * fx, p[1] / p[2] * fx);  // projected to screen
                fun(p_, cv::Scalar(0, 0, 255));
            }

            video_widget->update_image(frame_);
        }
    }
    qDebug()<<"Tracker:: Thread stopping";
}

cv::Vec3d Tracker_PT::get_model_offset()
{
    cv::Vec3d offset(s.t_MH_x, s.t_MH_y, s.t_MH_z);
    if (offset[0] == 0 && offset[1] == 0 && offset[2] == 0)
    {
        int m = s.model_used;
        switch (m)
        {
        default:
        // cap
        case 0: offset[0] = 0; offset[1] = 0; offset[2] = 0; break;
        // clip
        // new clip
        case 3:
        case 1: offset[0] = 135; offset[1] = 0; offset[2] = 0; break;
        // left clip
        case 4:
        case 2: offset[0] = -135; offset[1] = 0; offset[2] = 0; break;
        }
    }
    return offset;
}

void Tracker_PT::apply_settings()
{
    qDebug()<<"Tracker:: Applying settings";
    QMutexLocker l(&camera_mtx);
    camera.set_device_index(camera_name_to_index("PS3Eye Camera"));
    int res_x, res_y, cam_fps;
    switch (s.camera_mode)
    {
    default:
    case 0:
        res_x = 640;
        res_y = 480;
        cam_fps = 75;
        break;
    case 1:
        res_x = 640;
        res_y = 480;
        cam_fps = 60;
        break;
    case 2:
        res_x = 320;
        res_y = 240;
        cam_fps = 189;
        break;
    case 3:
        res_x = 320;
        res_y = 240;
        cam_fps = 120;
        break;
    }

    const CamInfo cur = camera.get_desired();

    if (cur.fps != cam_fps ||
        cur.res_x != res_x ||
        cur.res_y != res_y)
    {
        camera.set_res(res_x, res_y);
        camera.set_fps(cam_fps);
        qDebug() << "camera start";
        camera.start();
        frame = cv::Mat();
    }
    qDebug()<<"Tracker::apply ends";
}

void Tracker_PT::start_tracker(QFrame *parent_window)
{
    this->video_frame = parent_window;
    video_frame->setAttribute(Qt::WA_NativeWindow);
    video_frame->show();
    video_widget = new PTVideoWidget(video_frame);
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

        Affine X_MH(cv::Matx33d::eye(), get_model_offset());
        Affine X_GH = X_CM * X_MH;

        cv::Matx33d R = X_GH.R;
        cv::Vec3d   t = X_GH.t;

        // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
        // -z -> x, y -> z, x -> -y
        cv::Matx33d R_EG(0, 0,-1,
                         -1, 0, 0,
                         0, 1, 0);
        R = R_EG * R * R_EG.t();

        using std::atan2;
        using std::sqrt;

        // extract rotation angles
        double alpha, beta, gamma;
        beta  = atan2( -R(2,0), sqrt(R(2,1)*R(2,1) + R(2,2)*R(2,2)) );
        alpha = atan2( R(1,0), R(0,0));
        gamma = atan2( R(2,1), R(2,2));

        // extract rotation angles
        data[Yaw] = rad2deg * alpha;
        data[Pitch] = -rad2deg * beta;
        data[Roll] = rad2deg * gamma;
        // get translation(s)
        data[TX] = t[0] / 10.0;	// convert to cm
        data[TY] = t[1] / 10.0;
        data[TZ] = t[2] / 10.0;
    }
}
