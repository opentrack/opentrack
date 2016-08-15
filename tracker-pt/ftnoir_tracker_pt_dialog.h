/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_DIALOG_H
#define FTNOIR_TRACKER_PT_DIALOG_H

#include "api/plugin-api.hpp"
#include "ftnoir_tracker_pt_settings.h"
#include "ftnoir_tracker_pt.h"
#include "ui_FTNoIR_PT_Controls.h"
#include "cv/camera-dialog.hpp"
#include "cv/translation-calibrator.hpp"
#include "cv/video-widget.hpp"

#include <QTimer>

//-----------------------------------------------------------------------------
// The dialog that shows up when the user presses "Settings"
class TrackerDialog_PT : public ITrackerDialog, protected camera_dialog
{
    Q_OBJECT
public:
    TrackerDialog_PT();
    void register_tracker(ITracker *tracker) override;
    void unregister_tracker() override;
    void save();
public slots:
    void doOK();
    void doCancel();

    void startstop_trans_calib(bool start);
    void trans_calib_step();
    void poll_tracker_info();
    void camera_settings();
private:
    settings_pt s;
    Tracker_PT* tracker;
    QTimer timer, calib_timer;

    TranslationCalibrator trans_calib;
    bool trans_calib_running;

    Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_PT_DIALOG_H
