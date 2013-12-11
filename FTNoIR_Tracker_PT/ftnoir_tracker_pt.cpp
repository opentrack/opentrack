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
	: frame_count(0),
	  commands(0), 
	  video_widget(NULL), 
	  video_frame(NULL),
	  tracking_valid(false)
{
	qDebug()<<"Tracker::Tracker";
}

Tracker::~Tracker()
{
	qDebug()<<"Tracker::~Tracker";
	// terminate tracker thread
	set_command(ABORT);
	wait();
	// destroy video widget
	show_video_widget = false;
	update_show_video_widget();
}

void Tracker::set_command(Command command)
{
	QMutexLocker lock(&mutex);
	commands |= command;
}

void Tracker::reset_command(Command command)
{
	QMutexLocker lock(&mutex);
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
		{	
			QMutexLocker lock(&mutex);
			
			if (commands & ABORT) break;
			if (commands & PAUSE) continue;
			commands = 0;
			
			dt = time.elapsed() / 1000.0;
			time.restart();

			new_frame = camera.get_frame(dt, &frame);
			if (new_frame && !frame.empty())
			{
				frame = frame_rotation.rotate_frame(frame);
				const std::vector<cv::Vec2f>& points = point_extractor.extract_points(frame, dt, has_observers());
				tracking_valid = point_tracker.track(points, camera.get_info().f, dt);
				frame_count++;
#ifdef OPENTRACK_API
                video_widget->update_image(frame.clone());
#endif
			}
#ifdef PT_PERF_LOG
			log_stream<<"dt: "<<dt;
			if (!frame.empty()) log_stream<<" fps: "<<camera.get_info().fps;
			log_stream<<"\n";
#endif
		}
		msleep(sleep_time);
	}

	qDebug()<<"Tracker:: Thread stopping";
}

void Tracker::apply(const TrackerSettings& settings)
{
	qDebug()<<"Tracker:: Applying settings";
	QMutexLocker lock(&mutex);
	camera.set_device_index(settings.cam_index);
	camera.set_res(settings.cam_res_x, settings.cam_res_y);	
	camera.set_fps(settings.cam_fps);
	camera.set_f(settings.cam_f);
    frame_rotation.rotation = static_cast<RotationType>(settings.cam_roll);
	point_extractor.threshold_val = settings.threshold;
	point_extractor.threshold_secondary_val = settings.threshold_secondary;
	point_extractor.min_size = settings.min_point_size;
    point_extractor.max_size = settings.max_point_size;
    point_tracker.point_model = boost::shared_ptr<PointModel>(new PointModel(settings.M01, settings.M02));
    point_tracker.dynamic_pose_resolution = settings.dyn_pose_res;
	sleep_time = settings.sleep_time;
	point_tracker.dt_reset = settings.reset_time / 1000.0;
	show_video_widget = settings.video_widget;
	update_show_video_widget();
	bEnableRoll  = settings.bEnableRoll;
	bEnablePitch = settings.bEnablePitch;
	bEnableYaw   = settings.bEnableYaw;
	bEnableX     = settings.bEnableX;
	bEnableY     = settings.bEnableY;
	bEnableZ     = settings.bEnableZ;

	t_MH = settings.t_MH;
	R_GC =  Matx33f( cos(deg2rad*settings.cam_yaw), 0, sin(deg2rad*settings.cam_yaw),
		                                         0, 1,                             0,
		            -sin(deg2rad*settings.cam_yaw), 0, cos(deg2rad*settings.cam_yaw));
	R_GC = R_GC * Matx33f( 1,                                0,                               0,
		                   0,  cos(deg2rad*settings.cam_pitch), sin(deg2rad*settings.cam_pitch),
		                   0, -sin(deg2rad*settings.cam_pitch), cos(deg2rad*settings.cam_pitch));

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

void Tracker::update_show_video_widget()
{
	if (!show_video_widget && video_widget) {
		delete video_widget;
		video_widget = NULL;
		if (video_frame->layout()) delete video_frame->layout();
	}
	else if (video_frame && show_video_widget && !video_widget)
	{
        const int VIDEO_FRAME_WIDTH  = 320;
        const int VIDEO_FRAME_HEIGHT = 240;
        video_widget = new PTVideoWidget(video_frame, this);
		QHBoxLayout* video_layout = new QHBoxLayout();
		video_layout->setContentsMargins(0, 0, 0, 0);
		video_layout->addWidget(video_widget);
		video_frame->setLayout(video_layout);
		video_widget->resize(VIDEO_FRAME_WIDTH, VIDEO_FRAME_HEIGHT);
	}
}

//-----------------------------------------------------------------------------
// ITracker interface
void Tracker::Initialize(QFrame *video_frame)
{
	qDebug("Tracker::Initialize");
	// setup video frame	
	this->video_frame = video_frame;
	video_frame->setAttribute(Qt::WA_NativeWindow);
	video_frame->show();
	update_show_video_widget();
	TrackerSettings settings;
	settings.load_ini();
    camera.start();
    apply(settings);
	start();
}

void Tracker::refreshVideo()
{	
	if (video_widget) video_widget->update_frame_and_points();
}

#ifdef OPENTRACK_API
void Tracker::StartTracker(QFrame *parent_window)
#else
void Tracker::StartTracker(HWND parent_window)
#endif
{
#ifdef OPENTRACK_API
    Initialize(parent_window);
#endif
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

#ifndef OPENTRACK_API
		// get translation(s)
		if (bEnableX) data->x = t[0] / 10.0;	// convert to cm
		if (bEnableY) data->y = t[1] / 10.0;
		if (bEnableZ) data->z = t[2] / 10.0;
#else
        // get translation(s)
        if (bEnableX) data[TX] = t[0] / 10.0;	// convert to cm
        if (bEnableY) data[TY] = t[1] / 10.0;
        if (bEnableZ) data[TZ] = t[2] / 10.0;
#endif
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

#ifndef OPENTRACK_API
		if (bEnableYaw)   data->yaw   =   rad2deg * alpha;
		if (bEnablePitch) data->pitch = - rad2deg * beta;	// FTNoIR expects a minus here
		if (bEnableRoll)  data->roll  =   rad2deg * gamma;
#else
        if (bEnableYaw)   data[Yaw]  =   rad2deg * alpha;
        if (bEnablePitch) data[Pitch] = - rad2deg * beta;	// FTNoIR expects a minus here
        if (bEnableRoll)  data[Roll]  =   rad2deg * gamma;
#endif
	}
#ifndef OPENTRACK_API
    return true;
#endif
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
