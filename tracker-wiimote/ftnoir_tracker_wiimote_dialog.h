/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_WIIMOTE_DIALOG_H
#define FTNOIR_TRACKER_WIIMOTE_DIALOG_H

#include "opentrack/plugin-api.hpp"
#include "ftnoir_tracker_wiimote_settings.h"
#include "ftnoir_tracker_wiimote.h"
#include "trans_calib.h"
#include "wiimote_monitor_widget.h"
#include "ui_FTNoIR_WiiMote_Controls.h"
#include "opentrack/opencv-camera-dialog.hpp"

#include <QTimer>

//-----------------------------------------------------------------------------
// The dialog that shows up when the user presses "Settings"
class TrackerDialog_WiiMote : public ITrackerDialog, protected camera_dialog<Tracker_WiiMote>
{
    Q_OBJECT
public:
    TrackerDialog_WiiMote();
    void register_tracker(ITracker *tracker) override;
    void unregister_tracker() override;
    void save();
    void trans_calib_step();

public slots:
    void doOK();
    void doCancel();

    void startstop_trans_calib(bool start);
    void poll_tracker_info();
    void camera_settings();
private:
    settings_wiimote s;
    Tracker_WiiMote* tracker;
    QTimer timer;

    TranslationCalibrator trans_calib;
    bool trans_calib_running;

    Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_WIIMOTE_DIALOG_H
