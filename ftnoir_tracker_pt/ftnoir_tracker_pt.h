/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_H
#define FTNOIR_TRACKER_PT_H

#ifdef OPENTRACK_API
#   include "facetracknoir/plugin-api.hpp"
#endif
#include "ftnoir_tracker_pt_settings.h"
#include "camera.h"
#include "point_extractor.h"
#include "point_tracker.h"
#include "pt_video_widget.h"
#include "facetracknoir/timer.hpp"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QTime>
#include <opencv2/core/core.hpp>
#include <atomic>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include <memory>
#endif
#include <vector>

//-----------------------------------------------------------------------------
// Constantly processes the tracking chain in a separate thread
class Tracker : public ITracker, protected QThread
{
public:
    Tracker();
    ~Tracker() override;
    void StartTracker(QFrame* parent_window) override;
    void GetHeadPoseData(double* data) override;

    void apply(settings& s);
    void apply_inner();
    void center();
    void reset();	// reset the trackers internal state variables

    void get_pose(FrameTrafo* X_CM) { QMutexLocker lock(&mutex); *X_CM = point_tracker.get_pose(); }
    int  get_n_points() { QMutexLocker lock(&mutex); return point_extractor.get_points().size(); }
    void get_cam_info(CamInfo* info) { QMutexLocker lock(&mutex); *info = camera.get_info(); }
protected:
    void run() override;
private:
    QMutex mutex;
    // thread commands
    enum Command {
        ABORT = 1<<0
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

    settings s;
    std::atomic<settings*> new_settings;
    Timer time;

    static constexpr double rad2deg = 180.0/3.14159265;
    static constexpr double deg2rad = 3.14159265/180.0;

    PointModel model;
};

#undef VideoWidget

#endif // FTNOIR_TRACKER_PT_H
