/* Copyright (c) 2012 Patrick Ruoff
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

using namespace std;
using namespace cv;

//#define PT_PERF_LOG	//log performance

//-----------------------------------------------------------------------------
Tracker::Tracker()
    : mutex(QMutex::Recursive),
      commands(0),
	  video_widget(NULL),
	  video_frame(NULL)
{
    connect(s.b.get(), SIGNAL(saving()), this, SLOT(apply_settings()));
}

Tracker::~Tracker()
{
	set_command(ABORT);
	wait();
    delete video_widget;
    video_widget = NULL;
    if (video_frame->layout()) delete video_frame->layout();
}

void Tracker::set_command(Command command)
{
    //QMutexLocker lock(&mutex);
	commands |= command;
}

void Tracker::reset_command(Command command)
{
    //QMutexLocker lock(&mutex);
	commands &= ~command;
}

float Tracker::get_focal_length()
{
    static constexpr float pi = 3.1415926f;
    const float fov = static_cast<int>(s.fov) * pi / 180.f;
    return 0.5f / tan(0.5f * fov);
}

void Tracker::run()
{
#ifdef PT_PERF_LOG
	QFile log_file(QCoreApplication::applicationDirPath() + "/PointTrackerPerformance.txt");
	if (!log_file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
	QTextStream log_stream(&log_file);
#endif

    while((commands & ABORT) == 0)
    {
        const double dt = time.elapsed() * 1e-9;
        time.start();
        cv::Mat frame;
        const bool new_frame = camera.get_frame(dt, &frame);

        if (new_frame && !frame.empty())
        {
            QMutexLocker lock(&mutex);

            std::vector<cv::Vec2f> points = point_extractor.extract_points(frame);
            
            if (points.size() == PointModel::N_POINTS)
                point_tracker.track(points, PointModel(s), get_focal_length());
            
            {
                Affine CM = pose();
                cv::Vec3f MH(-s.t_MH_x, s.t_MH_y, s.t_MH_z);
                cv::Vec3f p = CM.t - MH;
                float fx = get_focal_length();
                cv::Vec2f p_(p[0] / p[2] * fx, p[1] / p[2] * fx);

                points.push_back(p_);
            }
            
            for (int i = 0; i < points.size(); i++)
            {
                auto& p = points[i];
                auto p2 = cv::Point(p[0] * frame.cols + frame.cols/2, -p[1] * frame.cols + frame.rows/2);
                cv::Scalar color(0, 255, 0);
                if (i == points.size()-1)
                    color = cv::Scalar(0, 0, 255);
                cv::line(frame,
                         cv::Point(p2.x - 20, p2.y),
                         cv::Point(p2.x + 20, p2.y),
                         color,
                         4);
                cv::line(frame,
                         cv::Point(p2.x, p2.y - 20),
                         cv::Point(p2.x, p2.y + 20),
                         color,
                         4);
            }
            
            video_widget->update_image(frame);
        }
#ifdef PT_PERF_LOG
        log_stream<<"dt: "<<dt;
        if (!frame.empty()) log_stream<<" fps: "<<camera.get_info().fps;
        log_stream<<"\n";
#endif
    }
    qDebug()<<"Tracker:: Thread stopping";
}

void Tracker::apply_settings()
{
    qDebug()<<"Tracker:: Applying settings";
    QMutexLocker lock(&mutex);
    camera.set_device_index(s.cam_index);
    camera.set_res(s.cam_res_x, s.cam_res_y);
    camera.set_fps(s.cam_fps);
    qDebug()<<"Tracker::apply ends";
}

void Tracker::start_tracker(QFrame *parent_window)
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
    apply_settings();
    camera.start();
    start();
}

#ifndef OPENTRACK_API
void Tracker::StopTracker(bool exit)
{
    set_command(PAUSE);
}
#endif

#ifdef OPENTRACK_API
#define THeadPoseData double
#endif

void Tracker::data(THeadPoseData *data)
{

    Affine X_CM = pose();

    Affine X_MH(Matx33f::eye(), cv::Vec3f(s.t_MH_x, s.t_MH_y, s.t_MH_z));
    Affine X_GH = X_CM * X_MH;

    Matx33f R = X_GH.R * X_MH.R.t();
    Vec3f   t = X_GH.t;

    // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
    // -z -> x, y -> z, x -> -y
    Matx33f R_EG(0, 0,-1,
                -1, 0, 0,
                 0, 1, 0);
    R = R_EG * R * R_EG.t();

    // extract rotation angles
    float alpha, beta, gamma;
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

//-----------------------------------------------------------------------------
#ifdef OPENTRACK_API
extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
#else
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")
OPENTRACK_EXPORT ITrackerPtr __stdcall GetTracker()
#endif
{
	return new Tracker;
}
