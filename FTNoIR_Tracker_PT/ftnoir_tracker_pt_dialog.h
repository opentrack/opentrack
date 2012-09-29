/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_DIALOG_H
#define FTNOIR_TRACKER_PT_DIALOG_H

#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "ftnoir_tracker_pt_settings.h"
#include "ftnoir_tracker_pt.h"
#include "ui_FTNoIR_PT_Controls.h"

//-----------------------------------------------------------------------------
class TrackerDialog : public QWidget, Ui::UICPTClientControls, public ITrackerDialog
{
    Q_OBJECT
public:
	TrackerDialog();
	~TrackerDialog();

	// ITrackerDialog interface
	void Initialize(QWidget *parent);
	void registerTracker(ITracker *tracker);
	void unRegisterTracker();

protected slots:
	void doOK();
	void doCancel();
	void doCenter();

	// ugly qt stuff
	void set_video_widget(bool val)  { settings.video_widget = val;   settings_changed(); }
	void set_sleep_time(int val)     { settings.sleep_time = val;     settings_changed(); }
	void set_cam_index(int val);
	void set_cam_f(double val)       { settings.cam_f = val;          settings_changed(); }
	void set_min_point_size(int val) { settings.min_point_size = val; settings_changed(); }
	void set_max_point_size(int val) { settings.max_point_size = val; settings_changed(); }
	void set_threshold(int val)      { settings.threshold = val;      settings_changed(); }
	void set_m1x(int val) { settings.M01[0] = val; settings_changed(); }
	void set_m1y(int val) { settings.M01[1] = val; settings_changed(); }
	void set_m1z(int val) { settings.M01[2] = val; settings_changed(); }
	void set_m2x(int val) { settings.M02[0] = val; settings_changed(); }
	void set_m2y(int val) { settings.M02[1] = val; settings_changed(); }
	void set_m2z(int val) { settings.M02[2] = val; settings_changed(); }

protected:
	void settings_changed();

	TrackerSettings settings;
	bool settings_dirty;

	Tracker* tracker;
	Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_PT_DIALOG_H