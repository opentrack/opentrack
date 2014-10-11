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
	  video_frame(NULL),
      new_settings(nullptr)
{
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

void Tracker::run()
{
#ifdef PT_PERF_LOG
	QFile log_file(QCoreApplication::applicationDirPath() + "/PointTrackerPerformance.txt");
	if (!log_file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
	QTextStream log_stream(&log_file);
#endif
    time.start();
	while((commands & ABORT) == 0)
	{   
        apply_inner();
        const double dt = time.start() * 1e-9;
        const bool new_frame = camera.get_frame(dt, &frame);

        if (new_frame && !frame.empty())
        {
            QMutexLocker lock(&mutex);

            frame = frame_rotation.rotate_frame(frame);
            const std::vector<cv::Vec2f>& points = point_extractor.extract_points(frame);
            for (auto p : points)
            {
                auto p2 = cv::Point(p[0] * frame.cols + frame.cols/2, -p[1] * frame.cols + frame.rows/2);
                cv::Scalar color(0, 255, 0);
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
            if (points.size() == PointModel::N_POINTS)
                point_tracker.track(points, model);
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
void Tracker::apply(settings& s)
{
    // caller guarantees object lifetime
	new_settings = &s;
}

void Tracker::apply_inner()
{
    settings* tmp = new_settings.exchange(nullptr);
    if (tmp == nullptr)
        return;
    reset();
    auto& s = *tmp;
    qDebug()<<"Tracker:: Applying settings";
    
    {
        cv::Vec3f M01(s.m01_x, s.m01_y, s.m01_z);
        cv::Vec3f M02(s.m02_x, s.m02_y, s.m02_z);
        model = PointModel(M01, M02);
    }
    camera.set_device_index(s.cam_index);
    camera.set_res(s.cam_res_x, s.cam_res_y);
    camera.set_fps(s.cam_fps);
    frame_rotation.rotation = static_cast<RotationType>(static_cast<int>(s.cam_roll));
    point_extractor.threshold_val = s.threshold;
    point_extractor.threshold_secondary_val = s.threshold_secondary;
    point_extractor.min_size = s.min_point_size;
    point_extractor.max_size = s.max_point_size;
    t_MH = cv::Vec3f(s.t_MH_x, s.t_MH_y, s.t_MH_z);
    R_GC =  Matx33f( cos(deg2rad*s.cam_yaw), 0, sin(deg2rad*s.cam_yaw),
		                                         0, 1,                             0,
                    -sin(deg2rad*s.cam_yaw), 0, cos(deg2rad*s.cam_yaw));
	R_GC = R_GC * Matx33f( 1,                                0,                               0,
                           0,  cos(deg2rad*s.cam_pitch), sin(deg2rad*s.cam_pitch),
                           0, -sin(deg2rad*s.cam_pitch), cos(deg2rad*s.cam_pitch));

	FrameTrafo X_MH(Matx33f::eye(), t_MH);
	X_GH_0 = R_GC * X_MH;
	qDebug()<<"Tracker::apply ends";
}

void Tracker::reset()
{
	QMutexLocker lock(&mutex);
	point_tracker.reset();
}

void Tracker::center()
{
	point_tracker.reset();	// we also do a reset here since there is no reset shortkey yet
	QMutexLocker lock(&mutex);
	FrameTrafo X_CM_0 = point_tracker.get_pose();
	FrameTrafo X_MH(Matx33f::eye(), t_MH);
	X_GH_0 = R_GC * X_CM_0 * X_MH;
}

void Tracker::StartTracker(QFrame *parent_window)
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
    camera.start();
	apply(s);
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

void Tracker::GetHeadPoseData(THeadPoseData *data)
{
	{
		QMutexLocker lock(&mutex);

    	FrameTrafo X_CM = point_tracker.get_pose();
		FrameTrafo X_MH(Matx33f::eye(), t_MH);
		FrameTrafo X_GH = R_GC * X_CM * X_MH;
        Matx33f R = X_GH.R * X_GH_0.R.t();
		Vec3f   t = X_GH.t - X_GH_0.t;		

        // get translation(s)
        data[TX] = t[0] / 10.0;	// convert to cm
        data[TY] = t[1] / 10.0;
        data[TZ] = t[2] / 10.0;

        // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
		// -z -> x, y -> z, x -> -y
		Matx33f R_EG( 0, 0,-1,
		             -1, 0, 0,
		              0, 1, 0);
		R = R_EG * R * R_EG.t();

		// extract rotation angles
		float alpha, beta, gamma;
		beta  = atan2( -R(2,0), sqrt(R(2,1)*R(2,1) + R(2,2)*R(2,2)) );
		alpha = atan2( R(1,0), R(0,0));
		gamma = atan2( R(2,1), R(2,2));		

        data[Yaw]   =   rad2deg * alpha;
        data[Pitch] = - rad2deg * beta;	// FTNoIR expects a minus here
        data[Roll]  =   rad2deg * gamma;
	}
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
