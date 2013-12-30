/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
*																				*
* Copyright (C) 2012	FuraX49 (HAT Tracker plugins)	    	     			*
* Homepage:			http://hatire.sourceforge.net								*
*																				*
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

#include <QScrollBar>

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
TrackerControls::TrackerControls() : theTracker(NULL), timer(this)
{

	ui.setupUi( this );

	ui.label_version->setText(VER_FILEVERSION_STR);

	// make SerialPort list
	ui.cbSerialPort->clear();
	foreach (QSerialPortInfo PortInfo , QSerialPortInfo::availablePorts() ) {
		ui.cbSerialPort->addItem(PortInfo.portName());
    }

	// Stop if no SerialPort dispo 
	if (ui.cbSerialPort->count()<1) {
		QMessageBox::critical(this,"FaceTrackNoIR Error", "No SerialPort avaible");
	} else {
        ui.cbSerialPort->setCurrentIndex(settings.SerialPortName);
	}
	// Serial config
	ui.QCB_Serial_baudRate->clear();
	ui.QCB_Serial_baudRate->addItem(QLatin1String("9600"),QSerialPort::Baud9600);
	ui.QCB_Serial_baudRate->addItem(QLatin1String("19200"),QSerialPort::Baud19200);
	ui.QCB_Serial_baudRate->addItem(QLatin1String("38400"),QSerialPort::Baud38400);
	ui.QCB_Serial_baudRate->addItem(QLatin1String("57600"),QSerialPort:: Baud57600);
	ui.QCB_Serial_baudRate->addItem(QLatin1String("115200"),QSerialPort::Baud115200);

	ui.QCB_Serial_dataBits->clear();
	ui.QCB_Serial_dataBits->addItem(QLatin1String("5"), QSerialPort::Data5);
	ui.QCB_Serial_dataBits->addItem(QLatin1String("6"), QSerialPort::Data6);
	ui.QCB_Serial_dataBits->addItem(QLatin1String("7"), QSerialPort::Data7);
	ui.QCB_Serial_dataBits->addItem(QLatin1String("8"), QSerialPort::Data8);

	ui.QCB_Serial_parity->clear();
	ui.QCB_Serial_parity->addItem(QLatin1String("None"), QSerialPort::NoParity);
	ui.QCB_Serial_parity->addItem(QLatin1String("Even"), QSerialPort::EvenParity);
	ui.QCB_Serial_parity->addItem(QLatin1String("Odd"), QSerialPort::OddParity);
	ui.QCB_Serial_parity->addItem(QLatin1String("Space"), QSerialPort::SpaceParity);
    ui.QCB_Serial_parity->addItem(QLatin1String("Mark"), QSerialPort::MarkParity);

	ui.QCB_Serial_stopBits->clear();
    ui.QCB_Serial_stopBits->addItem(QLatin1String("1"));
    ui.QCB_Serial_stopBits->addItem(QLatin1String("1.5"));
    ui.QCB_Serial_stopBits->addItem(QLatin1String("2"));


	ui.QCB_Serial_flowControl->clear();
    ui.QCB_Serial_flowControl->addItem(QLatin1String("None"));
    ui.QCB_Serial_flowControl->addItem(QLatin1String("RTS/CTS"));
    ui.QCB_Serial_flowControl->addItem(QLatin1String("XON/XOFF"));

    tie_setting(settings.EnableRoll, ui.chkEnableRoll);
    tie_setting(settings.EnablePitch, ui.chkEnablePitch);
    tie_setting(settings.EnableYaw, ui.chkEnableYaw);
    tie_setting(settings.EnableX, ui.chkEnableX);
    tie_setting(settings.EnableY, ui.chkEnableY);
    tie_setting(settings.EnableZ, ui.chkEnableZ);

    tie_setting(settings.InvertRoll, ui.chkInvertRoll);
    tie_setting(settings.InvertPitch, ui.chkInvertPitch);
    tie_setting(settings.InvertYaw, ui.chkInvertYaw);
    tie_setting(settings.InvertX, ui.chkInvertX);
    tie_setting(settings.InvertY, ui.chkInvertY);
    tie_setting(settings.InvertZ, ui.chkInvertZ);

    tie_setting(settings.RollAxe, ui.cb_roll);
    tie_setting(settings.RollAxe, ui.cb_roll);
    tie_setting(settings.RollAxe, ui.cb_roll);

    tie_setting(settings.XAxe, ui.cb_x);
    tie_setting(settings.YAxe, ui.cb_y);
    tie_setting(settings.ZAxe, ui.cb_z);

    tie_setting(settings.CmdStart, ui.le_cmd_start);
    tie_setting(settings.CmdStop, ui.le_cmd_stop);
    tie_setting(settings.CmdInit, ui.le_cmd_init);
    tie_setting(settings.CmdReset, ui.le_cmd_reset);
    tie_setting(settings.CmdCenter, ui.le_cmd_center);
    tie_setting(settings.CmdZero, ui.le_cmd_zero);

    tie_setting(settings.DelayInit, ui.spb_BeforeInit);
    tie_setting(settings.DelayStart, ui.spb_BeforeStart);
    tie_setting(settings.DelaySeq, ui.spb_AfterStart);

    tie_setting(settings.BigEndian, ui.cb_Endian);

    tie_setting(settings.pBaudRate, ui.QCB_Serial_baudRate);
    tie_setting(settings.pDataBits, ui.QCB_Serial_dataBits);
    tie_setting(settings.pParity, ui.QCB_Serial_parity);
    tie_setting(settings.pStopBits, ui.QCB_Serial_stopBits);
    tie_setting(settings.pFlowControl, ui.QCB_Serial_flowControl);

    tie_setting(settings.SerialPortName, ui.cbSerialPort);

	// Connect Qt signals to member-functions
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.btnSave, SIGNAL(clicked()), this, SLOT(doSave()));

	connect(ui.btnReset, SIGNAL(clicked()), this, SLOT(doReset()));
	connect(ui.btnCenter, SIGNAL(clicked()), this, SLOT(doCenter()));
	connect(ui.btnZero, SIGNAL(clicked()), this, SLOT(doZero()));
	connect(ui.btnSend, SIGNAL(clicked()), this, SLOT(doSend()));

	connect(ui.btn_icone, SIGNAL(clicked()), this, SLOT(doSerialInfo()));

	connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
}

//
// Destructor for server-dialog
//
TrackerControls::~TrackerControls() {
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
// Center asked to ARDUINO
//
void TrackerControls::doCenter() {
	if (theTracker) theTracker->notifyCenter();
}

//
// Zero asked to ARDUINO
//
void TrackerControls::doZero() {
	if (theTracker) theTracker->notifyZeroed();
}


//
// Reset asked to ARDUINO
//
void TrackerControls::doReset() {
	if (theTracker) theTracker->reset();
}


//
// Serial Info debug
//
void TrackerControls::doSerialInfo() {
	if (theTracker) theTracker->SerialInfo();
}


//
// Send command to ARDUINO
//
void TrackerControls::doSend() {
	if (theTracker) {
		if (!ui.lineSend->text().isEmpty()) {
			theTracker->sendcmd(ui.lineSend->text().toLatin1());
		}
	}
}


//
// Display FPS  of Arduino.
//
void TrackerControls::poll_tracker_info()
{
	if (theTracker)
	{	
		int nb_trame;

		theTracker->get_info(&nb_trame);
		ui.lab_vtps->setText(QString::number(nb_trame*(1000/last_time.elapsed())));
		last_time.restart();
	} 

}


void TrackerControls::WriteMsgInfo(const QByteArray &MsgInfo)
{
	QApplication::beep();
	ui.pteINFO->moveCursor(QTextCursor::End);
	ui.pteINFO->insertPlainText(QString(MsgInfo));
	QScrollBar *bar = ui.pteINFO->verticalScrollBar();
	bar->setValue(bar->maximum());
}



void TrackerControls::doSave() {
    settings.b->save();
    if (theTracker)
        theTracker->applysettings(settings);
}


//
// OK clicked on server-dialog
//
void TrackerControls::doOK() {
    settings.b->save();
    if (theTracker)
        theTracker->applysettings(settings);
	this->close();
}

//
// Cancel clicked on server-dialog
//
void TrackerControls::doCancel() {
	//
	// Ask if changed Settings should be saved
	//
    if (settings.b->modifiedp()) {
        int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
		switch (ret) {
			case QMessageBox::Save:
                settings.b->save();
				close();
				break;
			case QMessageBox::Discard:
                settings.b->revert();
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
	theTracker = static_cast<FTNoIR_Tracker*>(tracker);
	connect(theTracker, SIGNAL(sendMsgInfo(QByteArray)),this , SLOT(WriteMsgInfo(QByteArray)));

    if (isVisible() && settings.b->modifiedp()) theTracker->applysettings(settings);

	ui.cbSerialPort->setEnabled(false);
	ui.pteINFO->clear();
	ui.lab_vstatus->setText("HAT START");
	last_time.start();
	timer.start(250);

}


void TrackerControls::unRegisterTracker() {
	theTracker=NULL;
	timer.stop();
	ui.cbSerialPort->setEnabled(true);
	ui.lab_vstatus->setText("HAT STOPPED");
	ui.lab_vtps->setText("");
}

#ifdef OPENTRACK_API
extern "C" FTNOIR_TRACKER_BASE_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog( )
#else
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")
FTNOIR_TRACKER_BASE_EXPORT ITrackerDialogPtr __stdcall GetTrackerDialog( )
#endif
{
	return new TrackerControls;
}
