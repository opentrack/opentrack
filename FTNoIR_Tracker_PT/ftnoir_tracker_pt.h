/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_H
#define FTNOIR_TRACKER_PT_H

#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "ftnoir_tracker_pt_settings.h"
#include "camera.h"
#include "point_extractor.h"
#include "point_tracker.h"
#include "video_widget.h"
#include "timer.h"

#include <QThread>
#include <QMutex>
#include <QTime>
#include <opencv2/opencv.hpp>

//-----------------------------------------------------------------------------
class Tracker : public ITracker, QThread
{
public:
	Tracker();
	~Tracker();

	// ITracker interface
	void Initialize(QFrame *videoframe);
	void StartTracker(HWND parent_window);
	void StopTracker(bool exit);
	bool GiveHeadPoseData(THeadPoseData *data);
	void refreshVideo();

	void apply(const TrackerSettings& settings);
	void center();
	void reset();	// reset the trackers internal state variables
	void run();

	void get_pose(FrameTrafo* X_CM) { QMutexLocker lock(&mutex); *X_CM = point_tracker.get_pose(); }
	int get_n_points() { QMutexLocker lock(&mutex); return point_extractor.get_points().size(); }
	void get_cam_info(CamInfo* info) { QMutexLocker lock(&mutex); *info = camera.get_info(); }

protected:	
	FrameTrafo X_CH_0; // for centering

	QMutex mutex;
	cv::Mat frame;	// the output frame for display

	enum Command {
		ABORT = 1<<0,
		PAUSE = 1<<1
	};
	void set_command(Command command);
	void reset_command(Command command);
	int commands;
	
	VICamera camera;
	PointExtractor point_extractor;
	PointTracker point_tracker;
	cv::Vec3f t_MH;
	bool draw_frame;
	int sleep_time;
	
	long frame_count;

	VideoWidget* video_widget;
	Timer time;
};

#endif // FTNOIR_TRACKER_PT_H
