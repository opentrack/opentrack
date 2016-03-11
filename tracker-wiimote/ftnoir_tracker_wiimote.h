/* Copyright (c) 2012 Patrick Ruoff
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_WIIMOTE_H
#define FTNOIR_TRACKER_WIIMOTE_H

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QHBoxLayout>
#include <QDebug>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "opentrack/plugin-api.hpp"
#include "ftnoir_tracker_wiimote_settings.h"

#include "camera.h"
#include "point_tracker.h"
#include "wiimote_monitor_widget.h"
#include "opentrack-compat/sleep.hpp"
#include "opentrack-compat/timer.hpp"
#include "opentrack/opencv-camera-dialog.hpp"



class TrackerDialog_WiiMote;

//-----------------------------------------------------------------------------
// Constantly processes the tracking chain in a separate thread
class Tracker_WiiMote : public QThread, public ITracker
{
    Q_OBJECT
    friend class camera_dialog<Tracker_WiiMote>;
    friend class TrackerDialog_WiiMote;
public:
    Tracker_WiiMote();
    ~Tracker_WiiMote() override;
    void start_tracker(QFrame* parent_window) override;
    void data(double* data) override;

    Affine pose() { return point_tracker.pose(); }
    bool get_cam_info(CamInfo* info) { return camera.get_info(*info); }
    int get_n_points() { return num_points; };
protected:
    void run() override;
private:
    // thread commands
    enum Command {
        ABORT = 1<<0
    };
    void set_command(Command command);
    void reset_command(Command command);
    
    volatile int commands;
    volatile int num_points = 0;

    Camera       camera;
    PointTracker point_tracker;

    WiiMoteMonitorWidget* monitor_widget;
    QFrame*      monitor_frame;

    settings_wiimote s;
    
    volatile bool ever_success;
    std::vector<cv::Vec2f> points;
    
    // focal length for 42° horicontal FOV angle
    const float fx = 1.3025;

    static constexpr double rad2deg = 180.0/3.14159265;
    static constexpr double deg2rad = 3.14159265/180.0;
};

class TrackerDll : public Metadata
{
    QString name() { return QString("WiiMoteTracker 1.0"); }
    QIcon icon() { return QIcon(":/Resources/linux.png"); }
};

#endif // FTNOIR_TRACKER_WIIMOTE_H
