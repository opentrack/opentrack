/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
#include "ftnoir_tracker_hat.h"
#include "ftnoir_tracker_hat_dialog.h"

#include <QMessageBox>
#include <QDebug>

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
TrackerControls::TrackerControls() : settingsDirty(false), theTracker(NULL), timer(this),
QWidget()
{
	
    settingsDirty= false;
	theTracker = NULL;
	ui.setupUi( this );
	pre_trame = 0;
	settings.load_ini();

	// make SerialPort list
	foreach (QextPortInfo PortInfo , QextSerialEnumerator::getPorts() ) {
		ui.cbSerialPort->addItem(PortInfo.portName);
	} 
    // Stop if no SerialPort dispo 
	if (ui.cbSerialPort->count()<1) {
		QMessageBox::critical(this,"FaceTrackNoIR Error", "No SerialPort avaible");
	} else {

		int indxport =ui.cbSerialPort->findText(settings.SerialPortName,Qt::MatchExactly );
		if (indxport!=-1) {
			ui.cbSerialPort->setCurrentIndex(indxport);
		} else {
			QMessageBox::warning(this,"FaceTrackNoIR Error", "Selected SerialPort modified");
			ui.cbSerialPort-> setCurrentIndex(indxport);
		}
	}
  
	ui.chkEnableRoll->setChecked(settings.EnableRoll);
	ui.chkEnablePitch->setChecked(settings.EnablePitch);
	ui.chkEnableYaw->setChecked(settings.EnableYaw);
	ui.chkEnableX->setChecked(settings.EnableX);
	ui.chkEnableY->setChecked(settings.EnableY);
	ui.chkEnableZ->setChecked(settings.EnableZ);

	ui.chkInvertRoll->setChecked(settings.InvertRoll);
	ui.chkInvertPitch->setChecked(settings.InvertPitch);
	ui.chkInvertYaw->setChecked(settings.InvertYaw);
	ui.chkInvertX->setChecked(settings.InvertX);
	ui.chkInvertY->setChecked(settings.InvertY);
	ui.chkInvertZ->setChecked(settings.InvertZ);


	ui.cb_roll->setCurrentIndex(settings.RollAxe);
	ui.cb_pitch->setCurrentIndex(settings.PitchAxe);
	ui.cb_yaw->setCurrentIndex(settings.YawAxe);
	ui.cb_x->setCurrentIndex(settings.XAxe);
	ui.cb_y->setCurrentIndex(settings.YAxe);
	ui.cb_z->setCurrentIndex(settings.ZAxe);


	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(doSave()));


	connect(ui.cbSerialPort,  SIGNAL(currentIndexChanged(QString)), this,SLOT(set_mod_port(QString)) );

	connect( ui.chkEnableRoll,SIGNAL(toggled(bool)),		 this,SLOT(set_ena_roll(bool)) );
	connect( ui.chkEnablePitch,SIGNAL(toggled(bool)),		 this,SLOT(set_ena_pitch(bool)) );
	connect( ui.chkEnableYaw,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_yaw(bool)) );
	connect( ui.chkEnableX,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_x(bool)) );
	connect( ui.chkEnableY,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_y(bool)) );
	connect( ui.chkEnableZ,SIGNAL(toggled(bool)),			 this,SLOT(set_ena_z(bool)) );

	connect( ui.chkInvertRoll,SIGNAL(toggled(bool)),		 this,SLOT(set_inv_roll(bool)) );
	connect( ui.chkInvertPitch,SIGNAL(toggled(bool)),		 this,SLOT(set_inv_pitch(bool)) );
	connect( ui.chkInvertYaw,SIGNAL(toggled(bool)),			 this,SLOT(set_inv_yaw(bool)) );
	connect( ui.chkInvertX,SIGNAL(toggled(bool)),			 this,SLOT(set_inv_x(bool)) );
	connect( ui.chkInvertY,SIGNAL(toggled(bool)),			 this,SLOT(set_inv_y(bool)) );
	connect( ui.chkInvertZ,SIGNAL(toggled(bool)),			 this,SLOT(set_inv_z(bool)) );


	connect(ui.cb_roll, SIGNAL(currentIndexChanged(int)), this,SLOT(set_rot_roll(int)));
	connect(ui.cb_pitch, SIGNAL(currentIndexChanged(int)),this,SLOT(set_rot_pitch(int)));
	connect(ui.cb_yaw, SIGNAL(currentIndexChanged(int)),  this,SLOT(set_rot_yaw(int)));
	connect(ui.cb_x, SIGNAL(currentIndexChanged(int)),    this,SLOT(set_acc_x(int)));
	connect(ui.cb_y, SIGNAL(currentIndexChanged(int)),    this,SLOT(set_acc_y(int)));
	connect(ui.cb_z, SIGNAL(currentIndexChanged(int)),    this,SLOT(set_acc_z(int)));

	connect(ui.btnReset, SIGNAL(clicked()), this, SLOT(doReset()));
	connect(ui.btnCenter, SIGNAL(clicked()), this, SLOT(doCenter()));
	connect(ui.btnSend, SIGNAL(clicked()), this, SLOT(doSend()));

	connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
}

//
// Destructor for server-dialog
//
TrackerControls::~TrackerControls() {
    delete this;
}


//
// Initialize tracker-client-dialog
//
void TrackerControls::Initialize(QWidget *parent) {
	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}


//
// Apply online settings to tracker
//
void TrackerControls::settings_changed()
{
	settingsDirty = true;
	if (theTracker) theTracker->applysettings(settings);
}


//
// Center asked to ARDUINO
//
void TrackerControls::doCenter() {
	if (theTracker) theTracker->center();
}

//
// Reset asked to ARDUINO
//
void TrackerControls::doReset() {
	if (theTracker) theTracker->reset();
}


//
// Send command to ARDUINO
//
void TrackerControls::doSend() {
	if (theTracker) {
		if (!ui.lineSend->text().isEmpty()) {
			QString cmd;
			cmd=ui.lineSend->text();
  			theTracker->sendcmd(&cmd);
			ui.lineSend->clear();
		}
	}
}


//
// Display FPS and Status of Arduino.
//
void TrackerControls::poll_tracker_info()
{
	if (theTracker)
	{	
		QString info;
		int num_trame;
		int nb_trame;

		theTracker->get_info(&info,&num_trame);
		if ( !info.isNull()) {
			ui.lab_vstatus->setText(info);
			ui.pteINFO->moveCursor(QTextCursor::End);
			ui.pteINFO->insertPlainText(info);
		}


		if (pre_trame<num_trame)  
			{ nb_trame=num_trame-pre_trame;}
		else 
			{nb_trame=(1000-pre_trame)+num_trame;}
		ui.lab_vtps->setText(QString::number(nb_trame*(1000/timer.interval())));

		pre_trame=num_trame;
	} 
	
}


void TrackerControls::doSave() {
	settingsDirty=false;
    settings.save_ini();
}


//
// OK clicked on server-dialog
//
void TrackerControls::doOK() {
	settingsDirty=false;
    settings.save_ini();
	this->close();
}

//
// Cancel clicked on server-dialog
//
void TrackerControls::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

		//		qDebug() << "doCancel says: answer =" << ret;

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


void TrackerControls::registerTracker(ITracker *tracker) {
//	theTracker = (FTNoIR_Tracker *) tracker;	
	theTracker = static_cast<FTNoIR_Tracker*>(tracker);
	if (isVisible() && settingsDirty) theTracker->applysettings(settings);
	ui.cbSerialPort->setEnabled(false);
	timer.start(250);
	ui.lab_vstatus->setText("HAT START");
}


void TrackerControls::unRegisterTracker() {
	theTracker = NULL;
	ui.cbSerialPort->setEnabled(true);
	timer.stop();
	ui.lab_vstatus->setText("HAT STOPPED");
	ui.lab_vtps->setText("");
}




////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerDialogPtr __stdcall GetTrackerDialog( )
{
	return new TrackerControls;
}
