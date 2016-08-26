/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt_dialog.h"

#include "compat/camera-names.hpp"
#include <opencv2/core/core.hpp>
#include <memory>
#include <vector>
#include <QMessageBox>
#include <QString>
#include <QDebug>

//-----------------------------------------------------------------------------
TrackerDialog_PT::TrackerDialog_PT()
    : tracker(NULL),
      timer(this),
      trans_calib_running(false)
{
    ui.setupUi( this );

    ui.camdevice_combo->addItems(get_camera_names());

    tie_setting(s.camera_name, ui.camdevice_combo);
    tie_setting(s.cam_res_x, ui.res_x_spin);
    tie_setting(s.cam_res_y, ui.res_y_spin);
    tie_setting(s.cam_fps, ui.fps_spin);

    tie_setting(s.threshold, ui.threshold_slider);

    tie_setting(s.min_point_size, ui.mindiam_spin);
    tie_setting(s.max_point_size, ui.maxdiam_spin);

    tie_setting(s.clip_by, ui.clip_bheight_spin);
    tie_setting(s.clip_bz, ui.clip_blength_spin);
    tie_setting(s.clip_ty, ui.clip_theight_spin);
    tie_setting(s.clip_tz, ui.clip_tlength_spin);

    tie_setting(s.cap_x, ui.cap_width_spin);
    tie_setting(s.cap_y, ui.cap_height_spin);
    tie_setting(s.cap_z, ui.cap_length_spin);

    tie_setting(s.m01_x, ui.m1x_spin);
    tie_setting(s.m01_y, ui.m1y_spin);
    tie_setting(s.m01_z, ui.m1z_spin);

    tie_setting(s.m02_x, ui.m2x_spin);
    tie_setting(s.m02_y, ui.m2y_spin);
    tie_setting(s.m02_z, ui.m2z_spin);

    tie_setting(s.t_MH_x, ui.tx_spin);
    tie_setting(s.t_MH_y, ui.ty_spin);
    tie_setting(s.t_MH_z, ui.tz_spin);

    tie_setting(s.fov, ui.fov);

    tie_setting(s.active_model_panel, ui.model_tabs);

    tie_setting(s.dynamic_pose, ui.dynamic_pose);
    tie_setting(s.init_phase_timeout, ui.init_phase_timeout);

    tie_setting(s.auto_threshold, ui.auto_threshold);
    tie_setting(s.mean_shift_filter_blobs, ui.mean_shift_enable_checkbox);

    connect( ui.tcalib_button,SIGNAL(toggled(bool)), this,SLOT(startstop_trans_calib(bool)) );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
    connect(ui.camera_settings, SIGNAL(clicked()), this, SLOT(camera_settings()));
    timer.start(250);

    connect(&calib_timer, &QTimer::timeout, this, &TrackerDialog_PT::trans_calib_step);
    calib_timer.setInterval(100);
}

void TrackerDialog_PT::camera_settings()
{
    if (tracker)
        open_camera_settings(static_cast<cv::VideoCapture*>(tracker->camera), s.camera_name, &tracker->camera_mtx);
    else
        open_camera_settings(nullptr, s.camera_name, nullptr);
}

void TrackerDialog_PT::startstop_trans_calib(bool start)
{
    if (start)
    {
        qDebug()<<"TrackerDialog:: Starting translation calibration";
        calib_timer.start();
        trans_calib.reset();
        trans_calib_running = true;
        s.t_MH_x = 0;
        s.t_MH_y = 0;
        s.t_MH_z = 0;
    }
    else
    {
        calib_timer.stop();
        qDebug()<<"TrackerDialog:: Stopping translation calibration";
        trans_calib_running = false;
        {
            auto tmp = trans_calib.get_estimate();
            s.t_MH_x = tmp[0];
            s.t_MH_y = tmp[1];
            s.t_MH_z = tmp[2];
        }
    }
    ui.tx_spin->setEnabled(!start);
    ui.ty_spin->setEnabled(!start);
    ui.tz_spin->setEnabled(!start);
    ui.tcalib_button->setText(progn(
                                  if (start)
                                      return QStringLiteral("Stop calibration");
                                  else
                                      return QStringLiteral("Start calibration");
                                  ));
}

void TrackerDialog_PT::poll_tracker_info()
{
    CamInfo info;
    if (tracker && tracker->get_cam_info(&info))
    {
        QString to_print;
        {
            // display caminfo
            to_print = QString::number(info.res_x)+"x"+QString::number(info.res_y)+" @ "+QString::number(info.fps)+" FPS";
        }
        ui.caminfo_label->setText(to_print);

        // display pointinfo
        int n_points = tracker->get_n_points();
        to_print = QString::number(n_points);
        if (n_points == 3)
            to_print += " OK!";
        else
            to_print += " BAD!";
        ui.pointinfo_label->setText(to_print);

        // update calibration
        if (trans_calib_running) trans_calib_step();
    }
    else
    {
        ui.caminfo_label->setText("Tracker offline");
        ui.pointinfo_label->setText("");
    }
}

void TrackerDialog_PT::trans_calib_step()
{
    if (tracker)
    {
        Affine X_CM = tracker->pose();
        trans_calib.update(X_CM.R, X_CM.t);
    }
    else
        startstop_trans_calib(false);
}

void TrackerDialog_PT::save()
{
    s.b->save();
}

void TrackerDialog_PT::doOK()
{
    save();
    close();
}

void TrackerDialog_PT::doCancel()
{
    close();
}

void TrackerDialog_PT::register_tracker(ITracker *t)
{
    qDebug()<<"TrackerDialog:: Tracker registered";
    tracker = static_cast<Tracker_PT*>(t);
    ui.tcalib_button->setEnabled(true);
    //ui.center_button->setEnabled(true);
}

void TrackerDialog_PT::unregister_tracker()
{
    qDebug()<<"TrackerDialog:: Tracker un-registered";
    tracker = NULL;
    ui.tcalib_button->setEnabled(false);
    //ui.center_button->setEnabled(false);
}

