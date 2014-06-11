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
#include "facetracknoir/timer.hpp"

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
    virtual ~Tracker();
    virtual void StartTracker(QFrame* parent_window);
    virtual void GetHeadPoseData(double* data);
    virtual void refreshVideo();

    void apply(settings& s);
	void apply_inner();
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
    volatile int commands;

    CVCamera       camera;
	FrameRotation  frame_rotation;
	PointExtractor point_extractor;
	PointTracker   point_tracker;

	FrameTrafo X_GH_0; // for centering
	cv::Vec3f   t_MH; // translation from model frame to head frame
	cv::Matx33f R_GC; // rotation from opengl reference frame to camera frame

	// --- ui ---
	cv::Mat frame;	// the output frame for display

    PTVideoWidget* video_widget;
	QFrame*      video_frame;
    bool           tracking_valid, need_apply;
	
    settings s;
    settings* new_settings;
    Timer time;
};

#undef VideoWidget

#endif // FTNOIR_TRACKER_PT_H
