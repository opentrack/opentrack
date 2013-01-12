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
#include "trans_calib.h"

#include <QTimer>

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
	
	void trans_calib_step();

protected slots:
	// ugly qt stuff
	void set_video_widget(bool val)  { settings.video_widget = val;   settings_changed(); }
	void set_dyn_pose_res(bool val)  { settings.dyn_pose_res = val;   settings_changed(); }
	void set_sleep_time(int val)     { settings.sleep_time = val;     settings_changed(); }
	void set_reset_time(int val)     { settings.reset_time = val;     settings_changed(); }
	void set_cam_index(int val)		 { settings.cam_index = val;      settings_changed(); }
	void set_cam_f(double val)       { settings.cam_f = val;          settings_changed(); }
	void set_cam_res_x(int val)      { settings.cam_res_x = val;      settings_changed(); }
	void set_cam_res_y(int val)      { settings.cam_res_y = val;      settings_changed(); }
	void set_cam_fps(int val)        { settings.cam_fps = val;        settings_changed(); }
	void set_cam_pitch(int val)      { settings.cam_pitch = val;      settings_changed(); }
	void set_min_point_size(int val) { settings.min_point_size = val; settings_changed(); }
	void set_max_point_size(int val) { settings.max_point_size = val; settings_changed(); }
	void set_threshold(int val)      { settings.threshold = val;      settings_changed(); }
	void set_ena_roll(bool val)		 { settings.bEnableRoll = val;    settings_changed(); }
	void set_ena_pitch(bool val)	 { settings.bEnablePitch = val;   settings_changed(); }
	void set_ena_yaw(bool val)		 { settings.bEnableYaw = val;     settings_changed(); }
	void set_ena_x(bool val)		 { settings.bEnableX = val;       settings_changed(); }
	void set_ena_y(bool val)		 { settings.bEnableY = val;       settings_changed(); }
	void set_ena_z(bool val)		 { settings.bEnableZ = val;       settings_changed(); }

	void set_clip_t_height(int val)  { dialog_settings.clip_ty = val; set_clip(); }
	void set_clip_t_length(int val)  { dialog_settings.clip_tz = val; set_clip(); }
	void set_clip_b_height(int val)  { dialog_settings.clip_by = val; set_clip(); }
	void set_clip_b_length(int val)  { dialog_settings.clip_bz = val; set_clip(); }
	void set_cap_width(int val)      { dialog_settings.cap_x = val;   set_cap(); }
	void set_cap_height(int val)     { dialog_settings.cap_y = val;   set_cap(); }
	void set_cap_length(int val)     { dialog_settings.cap_z = val;   set_cap(); }
	void set_m1x(int val)            { dialog_settings.M01x = val;	  set_custom(); }
	void set_m1y(int val)            { dialog_settings.M01y = val;    set_custom(); }
	void set_m1z(int val)            { dialog_settings.M01z = val;    set_custom(); }
	void set_m2x(int val)            { dialog_settings.M02x = val;    set_custom(); }
	void set_m2y(int val)            { dialog_settings.M02y = val;    set_custom(); }
	void set_m2z(int val)            { dialog_settings.M02z = val;    set_custom(); }
	void set_tx(int val)             { settings.t_MH[0] = val;        settings_changed(); }
	void set_ty(int val)             { settings.t_MH[1] = val;        settings_changed(); }
	void set_tz(int val)             { settings.t_MH[2] = val;        settings_changed(); }
	void set_model(int model_id);

	void doCenter();
	void doReset();

	void doOK();
	void doCancel();

	void startstop_trans_calib(bool start);
	
	void poll_tracker_info();

protected:
	void set_clip();
	void set_cap();
	void set_custom();

	void settings_changed();

	TrackerSettings settings;
	TrackerDialogSettings dialog_settings;
	bool settings_dirty;

	Tracker* tracker;
	TranslationCalibrator trans_calib;
	bool trans_calib_running;
	QTimer timer;
	Ui::UICPTClientControls ui;
};

#endif //FTNOIR_TRACKER_PT_DIALOG_H