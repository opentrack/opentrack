/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2016 fred41
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_wiimote.h"

//-----------------------------------------------------------------------------
Tracker_WiiMote::Tracker_WiiMote()
    : commands(0),
      monitor_widget(NULL),
      monitor_frame(NULL),
      ever_success(false)
{
}

Tracker_WiiMote::~Tracker_WiiMote()
{
    set_command(ABORT);
    wait();
    if (monitor_widget)
        delete monitor_widget;
    monitor_widget = NULL;
    if (monitor_frame)
    {
        if (monitor_frame->layout()) delete monitor_frame->layout();
    }
}

void Tracker_WiiMote::set_command(Command command)
{
    commands |= command;
}

void Tracker_WiiMote::reset_command(Command command)
{
    commands &= ~command;
}

void Tracker_WiiMote::run()
{
    std::vector<cv::Vec2f> points;
    points.reserve(4);
    points.clear();

    Timer t_mon;
    t_mon.start();
    
    
    while((commands & ABORT) == 0)
    {
    
        bool freshpoints = camera.points_updated(points);
        
        while (!freshpoints && (commands & ABORT) == 0) {
            portable::sleep(3);
            freshpoints = camera.points_updated(points);
        
            // limit update() frequency
            if (t_mon.elapsed_ms() >= 20) {
                if (static_cast<QWidget*>(monitor_widget->parent())->isEnabled())
                    monitor_widget->update_image(points);
                t_mon.start();
            }
        }
        
        // for thread-extern monitoring
        num_points = points.size();

        if (points.size() >= PointModel::N_POINTS) {
            point_tracker.track(points, PointModel(s), fx, s.dynamic_pose, s.init_phase_timeout);
            if (!ever_success) ever_success = true;
        }     
 
    }

    qDebug()<<"Tracker:: Thread stopping";

}

void Tracker_WiiMote::start_tracker(QFrame *parent_window)
{
    this->monitor_frame = parent_window;
    monitor_frame->setAttribute(Qt::WA_NativeWindow);
    monitor_frame->show();
    monitor_widget = new WiiMoteMonitorWidget(monitor_frame);
    QHBoxLayout* monitor_layout = new QHBoxLayout(parent_window);
    monitor_layout->setContentsMargins(0, 0, 0, 0);
    monitor_layout->addWidget(monitor_widget);
    monitor_frame->setLayout(monitor_layout);
    monitor_widget->resize(monitor_frame->width(), monitor_frame->height());

    start();
}

void Tracker_WiiMote::data(double *data)
{
    if (ever_success)
    {
        Affine X_CM = pose();

        Affine X_MH(cv::Matx33f::eye(), cv::Vec3f(s.t_MH_x, s.t_MH_y, s.t_MH_z));
        Affine X_GH = X_CM * X_MH;

        cv::Matx33f R = X_GH.R;
        cv::Vec3f   t = X_GH.t;

        // translate rotation matrix from opengl (G) to roll-pitch-yaw (E) frame
        // -z -> x, y -> z, x -> -y
        cv::Matx33f R_EG(0, 0,-1,
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
}

#include "ftnoir_tracker_wiimote_dialog.h"
OPENTRACK_DECLARE_TRACKER(Tracker_WiiMote, TrackerDialog_WiiMote, TrackerDll)
