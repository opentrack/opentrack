/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_DIALOG_H
#define FTNOIR_TRACKER_PT_DIALOG_H

#ifdef OPENTRACK_API
#   include "facetracknoir/plugin-api.hpp"
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
class TrackerDialog : public QWidget, public ITrackerDialog
{
	Q_OBJECT
public:
	TrackerDialog();
	void registerTracker(ITracker *tracker) override;
	void unRegisterTracker() override;
    void save();
	void trans_calib_step();

public slots:
	void doOK();
	void doCancel();
    void doApply();
    void do_apply_without_saving(QAbstractButton *);

	void startstop_trans_calib(bool start);
	void poll_tracker_info();
    void set_model(int idx);
private:
	void set_model_clip();
	void set_model_cap();
	void set_model_custom();

	void settings_changed();

    settings s;
	Tracker* tracker;
	QTimer timer;

	TranslationCalibrator trans_calib;
	bool trans_calib_running;

	Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_PT_DIALOG_H
