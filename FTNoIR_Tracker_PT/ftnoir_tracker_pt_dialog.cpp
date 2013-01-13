/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt_dialog.h"

#include <QMessageBox>
#include <QDebug>

//-----------------------------------------------------------------------------
TrackerDialog::TrackerDialog()
	: settings_dirty(false), tracker(NULL), timer(this), trans_calib_running(false)
{
	qDebug()<<"TrackerDialog::TrackerDialog";
	setAttribute(Qt::WA_DeleteOnClose, false);

	ui.setupUi( this );

	settings.load_ini();
	dialog_settings.load_ini();

	// initialize ui values
	ui.videowidget_check->setChecked(settings.video_widget);
	ui.dynpose_check->setChecked(settings.dyn_pose_res);
	ui.sleep_spin->setValue(settings.sleep_time);
	ui.reset_spin->setValue(settings.reset_time);
	ui.camindex_spin->setValue(settings.cam_index);
	ui.f_dspin->setValue(settings.cam_f);
	ui.res_x_spin->setValue(settings.cam_res_x);
	ui.res_y_spin->setValue(settings.cam_res_y);
	ui.fps_spin->setValue(settings.cam_fps);
	ui.campitch_spin->setValue(settings.cam_pitch);
	ui.threshold_slider->setValue(settings.threshold);

	ui.chkEnableRoll->setChecked(settings.bEnableRoll);
	ui.chkEnablePitch->setChecked(settings.bEnablePitch);
	ui.chkEnableYaw->setChecked(settings.bEnableYaw);
	ui.chkEnableX->setChecked(settings.bEnableX);
	ui.chkEnableY->setChecked(settings.bEnableY);
	ui.chkEnableZ->setChecked(settings.bEnableZ);

	ui.mindiam_spin->setValue(settings.min_point_size);
	ui.maxdiam_spin->setValue(settings.max_point_size);
	ui.model_tabs->setCurrentIndex(dialog_settings.active_model_panel);
	ui.clip_bheight_spin->setValue(dialog_settings.clip_by);
	ui.clip_blength_spin->setValue(dialog_settings.clip_bz);
	ui.clip_theight_spin->setValue(dialog_settings.clip_ty);
	ui.clip_tlength_spin->setValue(dialog_settings.clip_tz);
	ui.cap_width_spin->setValue(dialog_settings.cap_x);
	ui.cap_height_spin->setValue(dialog_settings.cap_y);
	ui.cap_length_spin->setValue(dialog_settings.cap_z);
	ui.m1x_spin->setValue(dialog_settings.M01x);
	ui.m1y_spin->setValue(dialog_settings.M01y);
	ui.m1z_spin->setValue(dialog_settings.M01z);
	ui.m2x_spin->setValue(dialog_settings.M02x);
	ui.m2y_spin->setValue(dialog_settings.M02y);
	ui.m2z_spin->setValue(dialog_settings.M02z);
	ui.tx_spin->setValue(settings.t_MH[0]);
	ui.ty_spin->setValue(settings.t_MH[1]);
	ui.tz_spin->setValue(settings.t_MH[2]);

	// connect Qt signals and slots
	connect( ui.videowidget_check,SIGNAL(toggled(bool)),     this,SLOT(set_video_widget(bool)) );
	connect( ui.dynpose_check,SIGNAL(toggled(bool)),         this,SLOT(set_dyn_pose_res(bool)) );
	connect( ui.sleep_spin,SIGNAL(valueChanged(int)),        this,SLOT(set_sleep_time(int)) );
	connect( ui.reset_spin,SIGNAL(valueChanged(int)),        this,SLOT(set_reset_time(int)) );
	connect( ui.camindex_spin,SIGNAL(valueChanged(int)),     this,SLOT(set_cam_index(int)) );	
	connect( ui.f_dspin,SIGNAL(valueChanged(double)),        this,SLOT(set_cam_f(double)) );
	connect( ui.res_x_spin,SIGNAL(valueChanged(int)),        this,SLOT(set_cam_res_x(int)) );
	connect( ui.res_y_spin,SIGNAL(valueChanged(int)),        this,SLOT(set_cam_res_y(int)) );
	connect( ui.fps_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_cam_fps(int)) );
	connect( ui.campitch_spin,SIGNAL(valueChanged(int)),     this,SLOT(set_cam_pitch(int)) );
	connect( ui.threshold_slider,SIGNAL(sliderMoved(int)),   this,SLOT(set_threshold(int)) );

	connect( ui.chkEnableRoll,SIGNAL(toggled(bool)),		 this,SLOT(set_ena_roll(bool)) );
	connect( ui.chkEnablePitch,SIGNAL(toggled(bool)),		 this,SLOT(set_ena_pitch(bool)) );
	connect( ui.chkEnableYaw,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_yaw(bool)) );
	connect( ui.chkEnableX,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_x(bool)) );
	connect( ui.chkEnableY,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_y(bool)) );
	connect( ui.chkEnableZ,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_z(bool)) );

	connect( ui.mindiam_spin,SIGNAL(valueChanged(int)),      this,SLOT(set_min_point_size(int)) );
	connect( ui.maxdiam_spin,SIGNAL(valueChanged(int)),      this,SLOT(set_max_point_size(int)) );
	connect( ui.model_tabs,SIGNAL(currentChanged(int)),      this,SLOT(set_model(int)) );
	connect( ui.clip_theight_spin,SIGNAL(valueChanged(int)), this,SLOT(set_clip_t_height(int)) );
	connect( ui.clip_tlength_spin,SIGNAL(valueChanged(int)), this,SLOT(set_clip_t_length(int)) );
	connect( ui.clip_bheight_spin,SIGNAL(valueChanged(int)), this,SLOT(set_clip_b_height(int)) );
	connect( ui.clip_blength_spin,SIGNAL(valueChanged(int)), this,SLOT(set_clip_b_length(int)) );
	connect( ui.cap_width_spin,SIGNAL(valueChanged(int)),    this,SLOT(set_cap_width(int)) );
	connect( ui.cap_height_spin,SIGNAL(valueChanged(int)),   this,SLOT(set_cap_height(int)) );	
	connect( ui.cap_length_spin,SIGNAL(valueChanged(int)),   this,SLOT(set_cap_length(int)) );
	connect( ui.m1x_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_m1x(int)) );
	connect( ui.m1y_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_m1y(int)) );
	connect( ui.m1z_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_m1z(int)) );
	connect( ui.m2x_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_m2x(int)) );
	connect( ui.m2y_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_m2y(int)) );
	connect( ui.m2z_spin,SIGNAL(valueChanged(int)),          this,SLOT(set_m2z(int)) );
	connect( ui.tx_spin,SIGNAL(valueChanged(int)),           this,SLOT(set_tx(int)) );
	connect( ui.ty_spin,SIGNAL(valueChanged(int)),           this,SLOT(set_ty(int)) );
	connect( ui.tz_spin,SIGNAL(valueChanged(int)),           this,SLOT(set_tz(int)) );

	connect( ui.tcalib_button,SIGNAL(toggled(bool)), this,SLOT(startstop_trans_calib(bool)) );

	connect(ui.reset_button, SIGNAL(clicked()),  this, SLOT(doReset()));
	//connect(ui.center_button, SIGNAL(clicked()), this, SLOT(doCenter()));

	connect(ui.ok_button, SIGNAL(clicked()),     this, SLOT(doOK()));
	connect(ui.cancel_button, SIGNAL(clicked()), this, SLOT(doCancel()));

    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
    timer.start(100);
}

TrackerDialog::~TrackerDialog()
{
	qDebug()<<"TrackerDialog::~TrackerDialog";
}

void TrackerDialog::set_clip()
{
	settings.M01[0] =  0;
	settings.M01[1] =  dialog_settings.clip_ty;
	settings.M01[2] = -dialog_settings.clip_tz;
	settings.M02[0] =  0;
	settings.M02[1] = -dialog_settings.clip_by;
	settings.M02[2] = -dialog_settings.clip_bz;

	settings_changed();
}

void TrackerDialog::set_cap()
{
	settings.M01[0] = -dialog_settings.cap_x;
	settings.M01[1] = -dialog_settings.cap_y;
	settings.M01[2] = -dialog_settings.cap_z;
	settings.M02[0] =  dialog_settings.cap_x;
	settings.M02[1] = -dialog_settings.cap_y;
	settings.M02[2] = -dialog_settings.cap_z;

	settings_changed();
}

void TrackerDialog::set_custom()
{
	settings.M01[0] = dialog_settings.M01x;
	settings.M01[1] = dialog_settings.M01y;
	settings.M01[2] = dialog_settings.M01z;
	settings.M02[0] = dialog_settings.M02x;
	settings.M02[1] = dialog_settings.M02y;
	settings.M02[2] = dialog_settings.M02z;

	settings_changed();
}

void TrackerDialog::set_model(int val)
{
	dialog_settings.active_model_panel = val;

	switch (val) {

	case TrackerDialogSettings::MODEL_CLIP:
		set_clip();
		break;

	case TrackerDialogSettings::MODEL_CAP:
		set_cap();
		break;

	case TrackerDialogSettings::MODEL_CUSTOM:
		set_custom();
		break;

	default:
		break;
	}
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
		qDebug()<<"TrackerDialog:: Stoppping translation calibration";
		trans_calib_running = false;
		settings.t_MH = trans_calib.get_estimate();
		settings_changed();
	}
}

void TrackerDialog::trans_calib_step()
{
	if (tracker)
	{
		FrameTrafo X_CM;
		tracker->get_pose(&X_CM);
		trans_calib.update(X_CM.R, X_CM.t);
		cv::Vec3f t_MH = trans_calib.get_estimate();
		//qDebug()<<"TrackerDialog:: Current translation estimate: "<<t_MH[0]<<t_MH[1]<<t_MH[2];
		ui.tx_spin->setValue(t_MH[0]);
		ui.ty_spin->setValue(t_MH[1]);
		ui.tz_spin->setValue(t_MH[2]);
	}
}

void TrackerDialog::settings_changed()
{
	settings_dirty = true;
	if (tracker) tracker->apply(settings);
}

void TrackerDialog::doCenter()
{
	if (tracker) tracker->center();
}

void TrackerDialog::doReset()
{
	if (tracker) tracker->reset();
}

void TrackerDialog::doOK()
{
	settings.save_ini();
	dialog_settings.save_ini();
	close();
}

void TrackerDialog::doCancel()
{
	if (settings_dirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", 
			                              QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );
		switch (ret) {
			case QMessageBox::Save:
				settings.save_ini();
				dialog_settings.save_ini();
				close();
				break;
			case QMessageBox::Discard:
				close();
				break;
			case QMessageBox::Cancel:
				// Cancel was clicked
				break;
			default:
				// should never be reached
			break;
		}
	}
	else {
		close();
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
		ui.caminfo_label_2->setText(to_print);

		// display pointinfo
		int n_points = tracker->get_n_points();
		to_print = QString::number(n_points);
		if (n_points == 3)
			to_print += " OK!";
		else
			to_print += " BAD!";
		ui.pointinfo_label->setText(to_print);
		ui.pointinfo_label_2->setText(to_print);

		// update calibration
		if (trans_calib_running) trans_calib_step();
	}
	else
	{
		QString to_print = "Tracker offline";
		ui.caminfo_label->setText(to_print);
		ui.caminfo_label_2->setText(to_print);
		ui.pointinfo_label->setText(to_print);
		ui.pointinfo_label_2->setText(to_print);
	}
}


//-----------------------------------------------------------------------------
// ITrackerDialog interface
void TrackerDialog::Initialize(QWidget *parent)
{
	QPoint offsetpos(200, 200);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

void TrackerDialog::registerTracker(ITracker *t)
{
	qDebug()<<"TrackerDialog:: Tracker registered";
	tracker = static_cast<Tracker*>(t);
	if (isVisible() && settings_dirty) tracker->apply(settings);
	ui.tcalib_button->setEnabled(true);
	//ui.center_button->setEnabled(true);
	ui.reset_button->setEnabled(true);
}

void TrackerDialog::unRegisterTracker()
{
	qDebug()<<"TrackerDialog:: Tracker un-registered";
	tracker = NULL;
	ui.tcalib_button->setEnabled(false); 
	//ui.center_button->setEnabled(false);
	ui.reset_button->setEnabled(false);
}

//-----------------------------------------------------------------------------
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerDialogPtr __stdcall GetTrackerDialog( )
{
	return new TrackerDialog;
}