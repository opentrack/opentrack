/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_H
#define FTNOIR_TRACKER_PT_H

#ifdef OPENTRACK_API
#   include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#   include "facetracknoir/global-settings.h"
#endif
#include "ftnoir_tracker_pt_settings.h"
#include "frame_observer.h"
#include "camera.h"
#include "point_extractor.h"
#include "point_tracker.h"
#include "pt_video_widget.h"
#include "timer.h"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTime>
#include <opencv2/opencv.hpp>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include "FTNoIR_Tracker_PT/boost-compat.h"
#endif
#include <vector>

//-----------------------------------------------------------------------------
// Constantly processes the tracking chain in a separate thread
class Tracker : public ITracker, QThread, public FrameProvider
{
public:
	Tracker();
	~Tracker();

	// --- ITracker interface ---
	virtual void Initialize(QFrame *videoframe);
#ifdef OPENTRACK_API
    virtual void StartTracker(QFrame* parent_window);
    virtual bool GiveHeadPoseData(double* data);
#else
	virtual void StartTracker(HWND parent_window);
    virtual void StopTracker(bool exit);
    virtual bool GiveHeadPoseData(THeadPoseData *data);
#endif
    virtual void refreshVideo();

	void apply(const TrackerSettings& settings);
	void center();
	void reset();	// reset the trackers internal state variables
	void run();

	void get_pose(FrameTrafo* X_CM) { QMutexLocker lock(&mutex); *X_CM = point_tracker.get_pose(); }
	int  get_n_points() { QMutexLocker lock(&mutex); return point_extractor.get_points().size(); }
	void get_cam_info(CamInfo* info) { QMutexLocker lock(&mutex); *info = camera.get_info(); }

protected:
	// --- MutexedFrameProvider interface ---
	virtual bool get_frame_and_points(cv::Mat& frame, boost::shared_ptr< std::vector<cv::Vec2f> >& points);

	// --- thread ---
	QMutex mutex;
	// thread commands
	enum Command {
		ABORT = 1<<0,
		PAUSE = 1<<1
	};
	void set_command(Command command);
	void reset_command(Command command);
	int commands;	

	int sleep_time;	

	// --- tracking chain ---
#ifdef OPENTRACK_API
    CVCamera       camera;
#else
	VICamera       camera;
#endif
	FrameRotation  frame_rotation;
	PointExtractor point_extractor;
	PointTracker   point_tracker;
	bool           tracking_valid;

	FrameTrafo X_GH_0; // for centering
	cv::Vec3f   t_MH; // translation from model frame to head frame
	cv::Matx33f R_GC; // rotation from opengl reference frame to camera frame

	// --- ui ---
	cv::Mat frame;	// the output frame for display

	void update_show_video_widget();
	bool show_video_widget;
#ifdef OPENTRACK_API
    PTVideoWidget* video_widget;
#endif
	QFrame*      video_frame;

	// --- misc ---
	bool bEnableRoll;
	bool bEnablePitch;
	bool bEnableYaw;
	bool bEnableX;
	bool bEnableY;
	bool bEnableZ;
	
	long frame_count;
	Timer time;
};

#undef VideoWidget

#endif // FTNOIR_TRACKER_PT_H
