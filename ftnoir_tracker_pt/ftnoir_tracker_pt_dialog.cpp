/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt_dialog.h"

#include <QMessageBox>
#include <QDebug>
#include <opencv2/core/core.hpp>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include <memory>
#endif
#include <vector>

using namespace std;

//-----------------------------------------------------------------------------
TrackerDialog::TrackerDialog()
    : tracker(NULL),
      timer(this),
      trans_calib_running(false)
{
    ui.setupUi( this );

    vector<string> device_names;
    get_camera_device_names(device_names);
    for (vector<string>::iterator iter = device_names.begin(); iter != device_names.end(); ++iter)
    {
        ui.camdevice_combo->addItem(iter->c_str());
    }

    tie_setting(s.cam_index, ui.camdevice_combo);
    tie_setting(s.cam_res_x, ui.res_x_spin);
    tie_setting(s.cam_res_y, ui.res_y_spin);
    tie_setting(s.cam_fps, ui.fps_spin);

    tie_setting(s.threshold_secondary, ui.threshold_secondary_slider);
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

    connect( ui.tcalib_button,SIGNAL(toggled(bool)), this,SLOT(startstop_trans_calib(bool)) );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
    timer.start(100);
}

void TrackerDialog::startstop_trans_calib(bool start)
{
    if (start)
    {
        qDebug()<<"TrackerDialog:: Starting translation calibration";
        trans_calib.reset();
        trans_calib_running = true;
    }
    else
    {
        qDebug()<<"TrackerDialog:: Stopping translation calibration";
        trans_calib_running = false;
        {
            auto tmp = trans_calib.get_estimate();
            s.t_MH_x = tmp[0];
            s.t_MH_y = tmp[1];
            s.t_MH_z = tmp[2];
        }
    }
}

void TrackerDialog::poll_tracker_info()
{
    if (tracker)
    {
        QString to_print;

        // display caminfo
        CamInfo info;
        tracker->get_cam_info(&info);
        to_print = QString::number(info.res_x)+"x"+QString::number(info.res_y)+" @ "+QString::number(info.fps)+" FPS";
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
        QString to_print = "Tracker offline";
        ui.caminfo_label->setText(to_print);
        ui.pointinfo_label->setText(to_print);
    }
}

void TrackerDialog::trans_calib_step()
{
    if (tracker)
    {
        Affine X_CM = tracker->pose();
        trans_calib.update(X_CM.R, X_CM.t);
    }
}

void TrackerDialog::save()
{
    s.b->save();
}

void TrackerDialog::doOK()
{
    save();
    close();
}

void TrackerDialog::doCancel()
{
    s.b->reload();
    close();
}

void TrackerDialog::register_tracker(ITracker *t)
{
    qDebug()<<"TrackerDialog:: Tracker registered";
    tracker = static_cast<Tracker*>(t);
    ui.tcalib_button->setEnabled(true);
    //ui.center_button->setEnabled(true);
}

void TrackerDialog::unregister_tracker()
{
    qDebug()<<"TrackerDialog:: Tracker un-registered";
    tracker = NULL;
    ui.tcalib_button->setEnabled(false);
    //ui.center_button->setEnabled(false);
}

extern "C" OPENTRACK_EXPORT ITrackerDialog* GetDialog( )
{
    return new TrackerDialog;
}
