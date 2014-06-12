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
using namespace boost;

//#define PT_PERF_LOG	//log performance

const float rad2deg = 180.0/3.14159265;
const float deg2rad = 1.0/rad2deg;

//-----------------------------------------------------------------------------
Tracker::Tracker()
    : commands(0),
	  video_widget(NULL), 
	  video_frame(NULL),
	  tracking_valid(false),
	  need_apply(false),
	  new_settings(nullptr)
{
	qDebug()<<"Tracker::Tracker";
}

Tracker::~Tracker()
{
	qDebug()<<"Tracker::~Tracker";
	// terminate tracker thread
	set_command(ABORT);
	wait();
    s.video_widget = false;
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
	qDebug()<<"Tracker:: Thread started";

#ifdef PT_PERF_LOG
	QFile log_file(QCoreApplication::applicationDirPath() + "/PointTrackerPerformance.txt");
	if (!log_file.open(QIODevice::WriteOnly | QIODevice::Text)) return;
	QTextStream log_stream(&log_file);
#endif

	time.start();
	float dt;
	bool new_frame;
	forever
	{   
        if (commands & ABORT) break;
        if (commands & PAUSE) continue;
        commands = 0;
	apply_inner();
        dt = time.start() / 1000.0;

        new_frame = camera.get_frame(dt, &frame);

        if (new_frame && !frame.empty())
        {
            QMutexLocker lock(&mutex);

            frame = frame_rotation.rotate_frame(frame);
            const std::vector<cv::Vec2f>& points = point_extractor.extract_points(frame, dt, true);
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
            tracking_valid = point_tracker.track(points, camera.get_info().f, dt);
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
	QMutexLocker lock(&mutex);
	need_apply = true;
	// caller guarantees object lifetime
	new_settings = &s;
}

void Tracker::apply_inner()
{
	QMutexLocker lock(&mutex);
	if (!need_apply)
		return;
	qDebug()<<"Tracker:: Applying settings";
	auto& s = *new_settings;
	new_settings = nullptr;
	need_apply = false;
    camera.set_device_index(s.cam_index);
    camera.set_res(s.cam_res_x, s.cam_res_y);
    camera.set_fps(s.cam_fps);
    camera.set_f(s.cam_f);
    frame_rotation.rotation = static_cast<RotationType>(static_cast<int>(s.cam_roll));
    point_extractor.threshold_val = s.threshold;
    point_extractor.threshold_secondary_val = s.threshold_secondary;
    point_extractor.min_size = s.min_point_size;
    point_extractor.max_size = s.max_point_size;
    {
        cv::Vec3f M01(s.m01_x, s.m01_y, s.m01_z);
        cv::Vec3f M02(s.m02_x, s.m02_y, s.m02_z);
        point_tracker.point_model = boost::shared_ptr<PointModel>(new PointModel(M01, M02));
    }
    point_tracker.dynamic_pose_resolution = s.dyn_pose_res;
    point_tracker.dt_reset = s.reset_time / 1000.0;
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

bool Tracker::get_frame_and_points(cv::Mat& frame_copy, boost::shared_ptr< std::vector<Vec2f> >& points)
{
	QMutexLocker lock(&mutex);
	if (frame.empty()) return false;
		
	// copy the frame and points from the tracker thread
	frame_copy = frame.clone();
	points = boost::shared_ptr< vector<Vec2f> >(new vector<Vec2f>(point_extractor.get_points()));
	return true;
}

void Tracker::refreshVideo()
{	
	if (video_widget) video_widget->update_frame_and_points();
}

void Tracker::StartTracker(QFrame *parent_window)
{
    this->video_frame = parent_window;
    video_frame->setAttribute(Qt::WA_NativeWindow);
    video_frame->show();
    video_widget = new PTVideoWidget(video_frame, this);
    QHBoxLayout* video_layout = new QHBoxLayout(parent_window);
    video_layout->setContentsMargins(0, 0, 0, 0);
    video_layout->addWidget(video_widget);
    video_frame->setLayout(video_layout);
    video_widget->resize(video_frame->width(), video_frame->height());
    camera.start();
	apply(s);
    start();
    reset_command(PAUSE);
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

        if (!tracking_valid) return;

		FrameTrafo X_CM = point_tracker.get_pose();
		FrameTrafo X_MH(Matx33f::eye(), t_MH);
		FrameTrafo X_GH = R_GC * X_CM * X_MH;
        Matx33f R = X_GH.R * X_GH_0.R.t();
		Vec3f   t = X_GH.t - X_GH_0.t;		

        // get translation(s)
        if (s.bEnableX) data[TX] = t[0] / 10.0;	// convert to cm
        if (s.bEnableY) data[TY] = t[1] / 10.0;
        if (s.bEnableZ) data[TZ] = t[2] / 10.0;

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

        if (s.bEnableYaw)   data[Yaw]  =   rad2deg * alpha;
        if (s.bEnablePitch) data[Pitch] = - rad2deg * beta;	// FTNoIR expects a minus here
        if (s.bEnableRoll)  data[Roll]  =   rad2deg * gamma;
	}
}

//-----------------------------------------------------------------------------
#ifdef OPENTRACK_API
extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
#else
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")
FTNOIR_TRACKER_BASE_EXPORT ITrackerPtr __stdcall GetTracker()
#endif
{
	return new Tracker;
}
