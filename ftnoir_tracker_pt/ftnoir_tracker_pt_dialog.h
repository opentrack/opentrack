/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_DIALOG_H
#define FTNOIR_TRACKER_PT_DIALOG_H

#ifdef OPENTRACK_API
#   include "opentrack/plugin-api.hpp"
#else
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#endif
#include "ftnoir_tracker_pt_settings.h"
#include "ftnoir_tracker_pt.h"
#include "trans_calib.h"
#include "pt_video_widget.h"
#include "ui_FTNoIR_PT_Controls.h"

#include <QTimer>

//-----------------------------------------------------------------------------
// The dialog that shows up when the user presses "Settings"
class TrackerDialog_PT : public ITrackerDialog
{
	Q_OBJECT
public:
	TrackerDialog_PT();
	void register_tracker(ITracker *tracker) override;
	void unregister_tracker() override;
    void save();
	void trans_calib_step();

public slots:
	void doOK();
	void doCancel();

	void startstop_trans_calib(bool start);
	void poll_tracker_info();
private:
    settings_pt s;
	Tracker_PT* tracker;
	QTimer timer;

	TranslationCalibrator trans_calib;
	bool trans_calib_running;

	Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_PT_DIALOG_H
