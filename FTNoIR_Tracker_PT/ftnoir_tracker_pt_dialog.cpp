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
	ui.setupUi( this );

	settings.load_ini();

	// initialize ui values
	ui.videowidget_check->setChecked(settings.video_widget);
	ui.sleep_spin->setValue(settings.sleep_time);
	ui.camindex_spin->setValue(settings.cam_index);
	ui.f_dspin->setValue(settings.cam_f);
	ui.threshold_slider->setValue(settings.threshold);
	ui.mindiam_spin->setValue(settings.min_point_size);
	ui.maxdiam_spin->setValue(settings.max_point_size);

	ui.m1x_spin->setValue(settings.M01[0]);
	ui.m1y_spin->setValue(settings.M01[1]);
	ui.m1z_spin->setValue(settings.M01[2]);
	ui.m2x_spin->setValue(settings.M02[0]);
	ui.m2y_spin->setValue(settings.M02[1]);
	ui.m2z_spin->setValue(settings.M02[2]);
	ui.tx_spin->setValue(settings.t_MH[0]);
	ui.ty_spin->setValue(settings.t_MH[1]);
	ui.tz_spin->setValue(settings.t_MH[2]);

	ui.tcalib_button->setEnabled(false); 
	
	// connect Qt signals and slots
	connect( ui.videowidget_check,SIGNAL(toggled(bool)),   this,SLOT(set_video_widget(bool)) );
	connect( ui.sleep_spin,SIGNAL(valueChanged(int)),      this,SLOT(set_sleep_time(int)) );
	connect( ui.camindex_spin,SIGNAL(valueChanged(int)),   this,SLOT(set_cam_index(int)) );	
	connect( ui.f_dspin,SIGNAL(valueChanged(double)),      this,SLOT(set_cam_f(double)) );
	connect( ui.threshold_slider,SIGNAL(sliderMoved(int)), this,SLOT(set_threshold(int)) );
	connect( ui.mindiam_spin,SIGNAL(valueChanged(int)),    this,SLOT(set_min_point_size(int)) );
	connect( ui.maxdiam_spin,SIGNAL(valueChanged(int)),    this,SLOT(set_max_point_size(int)) );

	connect( ui.m1x_spin,SIGNAL(valueChanged(int)), this,SLOT(set_m1x(int)) );
	connect( ui.m1y_spin,SIGNAL(valueChanged(int)), this,SLOT(set_m1y(int)) );
	connect( ui.m1z_spin,SIGNAL(valueChanged(int)), this,SLOT(set_m1z(int)) );
	connect( ui.m2x_spin,SIGNAL(valueChanged(int)), this,SLOT(set_m2x(int)) );
	connect( ui.m2y_spin,SIGNAL(valueChanged(int)), this,SLOT(set_m2y(int)) );
	connect( ui.m2z_spin,SIGNAL(valueChanged(int)), this,SLOT(set_m2z(int)) );
	connect( ui.tx_spin,SIGNAL(valueChanged(int)), this,SLOT(set_tx(int)) );
	connect( ui.ty_spin,SIGNAL(valueChanged(int)), this,SLOT(set_ty(int)) );
	connect( ui.tz_spin,SIGNAL(valueChanged(int)), this,SLOT(set_tz(int)) );

	connect( ui.tcalib_button,SIGNAL(toggled(bool)), this,SLOT(startstop_trans_calib(bool)) );

	connect(ui.ok_button, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.cancel_button, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.center_button, SIGNAL(clicked()), this, SLOT(doCenter()));

    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
    timer.start(100);
}

void TrackerDialog::startstop_trans_calib(bool start)
{
	if (start)
	{
		qDebug()<<"TrackerDialog:: starting translation calibration";
		trans_calib.reset();
		trans_calib_running = true;
	}
	else
	{
		qDebug()<<"TrackerDialog:: stoppping translation calibration";
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
		qDebug()<<"TrackerDialog:: current translation estimate: "<<t_MH[0]<<t_MH[1]<<t_MH[2];
		ui.tx_spin->setValue(t_MH[0]);
		ui.ty_spin->setValue(t_MH[1]);
		ui.tz_spin->setValue(t_MH[2]);
	}
}

void TrackerDialog::set_cam_index(int val)
{	
	settings.cam_index = val;
	settings_dirty = true;
	if (tracker)
		tracker->apply(settings);
}

void TrackerDialog::settings_changed()
{
	settings_dirty = true;
	if (tracker)
		tracker->apply_without_camindex(settings);
}

void TrackerDialog::doCenter()
{
	if (tracker)
		tracker->CenterTracker();
}

void TrackerDialog::doOK()
{
	settings.save_ini();
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
		CamInfo info;
		tracker->get_cam_info(&info);
		ui.caminfo_label->setText(QString::number(info.res_x)+"x"+QString::number(info.res_y)+" @ "+QString::number(info.fps)+" FPS");

		int n_points = tracker->get_n_points();
		QString to_print = QString::number(n_points);
		if (n_points == 3)
			to_print += " OK!";
		else
			to_print += " BAD!";
		ui.pointinfo_label->setText(to_print);

		if (trans_calib_running) trans_calib_step();
	}
	else
	{
		ui.caminfo_label->setText("Tracker offline");
		ui.pointinfo_label->setText("Tracker offline");
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
	qDebug()<<"tracker registerd";
	tracker = static_cast<Tracker*>(t);
	if (isVisible() && settings_dirty)
		tracker->apply(settings);
	ui.tcalib_button->setEnabled(true); 
}

void TrackerDialog::unRegisterTracker()
{
	qDebug()<<"tracker un-registerd";
	tracker = NULL;
	ui.tcalib_button->setEnabled(false); 
}

//-----------------------------------------------------------------------------
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERDIALOGHANDLE __stdcall GetTrackerDialog( )
{
	return new TrackerDialog;
}