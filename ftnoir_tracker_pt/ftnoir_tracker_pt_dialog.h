/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_DIALOG_H
#define FTNOIR_TRACKER_PT_DIALOG_H

#ifdef OPENTRACK_API
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
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
class TrackerDialog : public QWidget, Ui::UICPTClientControls, public ITrackerDialog
{
	Q_OBJECT
public:
	TrackerDialog();
	void registerTracker(ITracker *tracker);
	void unRegisterTracker();
    void save();
	void trans_calib_step();

public slots:
    void doCenter();
    void doReset();
	void doOK();
    void doApply();
	void doCancel();
    void do_apply_without_saving();

	void startstop_trans_calib(bool start);
	void widget_destroyed(QObject* obj);
	void create_video_widget();
	void poll_tracker_info();
    void set_model(int idx);

protected:
	void destroy_video_widget(bool do_delete = true);

	void set_model_clip();
	void set_model_cap();
	void set_model_custom();

	void settings_changed();

    settings s;
	Tracker* tracker;
    VideoWidgetDialog* video_widget_dialog;
	QTimer timer;

	TranslationCalibrator trans_calib;
	bool trans_calib_running;

	Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_PT_DIALOG_H
