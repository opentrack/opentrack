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

using namespace std;
using namespace cv;
using namespace boost;

//-----------------------------------------------------------------------------
Tracker::Tracker()
	: frame_count(0), commands(0), video_widget(NULL)
{
	qDebug()<<"Tracker Const";
	TrackerSettings settings;
	settings.load_ini();
	apply(settings);
	qDebug()<<"Tracker Starting";
	start();
}

Tracker::~Tracker()
{
	set_command(ABORT);
	wait();
	if (video_widget) delete video_widget;
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
	qDebug()<<"Tracker Thread started";
	time.start();
	forever
	{
		{	
			QMutexLocker lock(&mutex);
			
			if (commands & ABORT) break;
			if (commands & PAUSE) continue;
			commands = 0;
			
			float dt = time.elapsed() / 1000.0;
			time.restart();

			frame = camera.get_frame(dt);
			if (!frame.empty())
			{
				const std::vector<cv::Vec2f>& points = point_extractor.extract_points(frame, dt, draw_frame);
				point_tracker.track(points, camera.get_info().f, dt);
				frame_count++;
			}
		}
		msleep(sleep_time);
	}
	qDebug()<<"Tracker Thread stopping";
}

void Tracker::apply(const TrackerSettings& settings)
{
	apply_without_camindex(settings);
	QMutexLocker lock(&mutex);
	qDebug()<<"Tracker: setting cam index "<<settings.cam_index;
	camera.set_index(settings.cam_index);
	qDebug()<<"Tracker: done setting cam index";
}

void Tracker::apply_without_camindex(const TrackerSettings& settings)
{
	qDebug()<<"Tracker::apply_without_camindex";
	QMutexLocker lock(&mutex);
	camera.set_f(settings.cam_f);
	point_extractor.threshold_val = settings.threshold;
	point_extractor.min_size = settings.min_point_size;
	point_extractor.max_size = settings.max_point_size;
	point_tracker.point_model = boost::shared_ptr<PointModel>(new PointModel(settings.M01, settings.M02));
	sleep_time = settings.sleep_time;
	draw_frame = settings.video_widget;
	t_MH = settings.t_MH;
}

//-----------------------------------------------------------------------------
// ITracker interface
void Tracker::Initialize(QFrame *videoframe)
{
	const int VIDEO_FRAME_WIDTH = 252;
	const int VIDEO_FRAME_HEIGHT = 189;

	qDebug("Tracker::Initialize()");
	// setup video frame
	videoframe->setAttribute(Qt::WA_NativeWindow);
	videoframe->show();
	video_widget = new VideoWidget(videoframe);
	QHBoxLayout* layout = new QHBoxLayout();
	layout->setContentsMargins(0, 0, 0, 0);
	layout->addWidget(video_widget);
	if (videoframe->layout()) delete videoframe->layout();
	videoframe->setLayout(layout);
	video_widget->resize(VIDEO_FRAME_WIDTH, VIDEO_FRAME_HEIGHT);
}

void Tracker::refreshVideo()
{	
	if (video_widget)
	{
		Mat frame_copy;
		shared_ptr< vector<Vec2f> > points;
		//qDebug("Tracker::refreshVideo()");
		{
			QMutexLocker lock(&mutex);
			if (!draw_frame || frame.empty()) return;
		
			// copy the frame and points from the tracker thread
			frame_copy = frame.clone();
			points = shared_ptr< vector<Vec2f> >(new vector<Vec2f>(point_extractor.get_points()));
		}
		
		video_widget->update(frame_copy, points);
	}
}

void Tracker::StartTracker(HWND parent_window)
{
	reset_command(PAUSE);
}

void Tracker::StopTracker(bool exit)
{
	set_command(PAUSE);
}

bool Tracker::GiveHeadPoseData(THeadPoseData *data)
{
	const float rad2deg = 180.0/3.14159265;
	{
		QMutexLocker lock(&mutex);
		FrameTrafo pose = point_tracker.get_pose();

		// get translation
		Vec3f p = pose.R * t_MH + pose.t;
		data->x = p[0] / 10.0;	// convert to cm
		data->y = p[1] / 10.0;
		data->z = p[2] / 10.0;

		// extract rotation angles from rotation matrix
		const Matx33f& R = pose.R;
		data->yaw = atan2( -R(2,0), sqrt(R(0,0)*R(0,0) + R(1,0)*R(1,0)) );
		float cos_beta = cos(data->yaw);
		if (cos_beta != 0)
		{
			data->pitch = rad2deg * atan2( R(2,1)/cos_beta, R(2,2)/cos_beta);
			data->roll  = rad2deg * atan2( R(1,0)/cos_beta, R(0,0)/cos_beta);
		}
		data->yaw *= rad2deg;
	}
	refreshVideo();
	return true;
}

//-----------------------------------------------------------------------------
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERHANDLE __stdcall GetTracker()
{
	return new Tracker;
}