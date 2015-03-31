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
TrackerControls::TrackerControls() : theTracker(NULL), settingsDirty(false), timer(this)
{

    ui.setupUi( this );
    settings.load_ini();

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

        int indxport =ui.cbSerialPort->findText(settings.SerialPortName,Qt::MatchExactly );
        if (indxport!=-1) {
            ui.cbSerialPort->setCurrentIndex(indxport);
        } else {
            QMessageBox::warning(this,"FaceTrackNoIR Error", "Selected SerialPort modified");
            ui.cbSerialPort-> setCurrentIndex(indxport);
        }
    }
    // Serial config
    ui.QCB_Serial_baudRate->clear();
    ui.QCB_Serial_baudRate->addItem(QLatin1String("9600"),QSerialPort::Baud9600);
    ui.QCB_Serial_baudRate->addItem(QLatin1String("19200"),QSerialPort::Baud19200);
    ui.QCB_Serial_baudRate->addItem(QLatin1String("38400"),QSerialPort::Baud38400);
    ui.QCB_Serial_baudRate->addItem(QLatin1String("57600"),QSerialPort:: Baud57600);
    ui.QCB_Serial_baudRate->addItem(QLatin1String("115200"),QSerialPort::Baud115200);
    ui.QCB_Serial_baudRate->setCurrentIndex(ui.QCB_Serial_baudRate->findData(settings.pBaudRate));

    ui.QCB_Serial_dataBits->clear();
    ui.QCB_Serial_dataBits->addItem(QLatin1String("5"), QSerialPort::Data5);
    ui.QCB_Serial_dataBits->addItem(QLatin1String("6"), QSerialPort::Data6);
    ui.QCB_Serial_dataBits->addItem(QLatin1String("7"), QSerialPort::Data7);
    ui.QCB_Serial_dataBits->addItem(QLatin1String("8"), QSerialPort::Data8);
    ui.QCB_Serial_dataBits->setCurrentIndex(ui.QCB_Serial_dataBits->findData(settings.pDataBits));

    ui.QCB_Serial_parity->clear();
    ui.QCB_Serial_parity->addItem(QLatin1String("None"), QSerialPort::NoParity);
    ui.QCB_Serial_parity->addItem(QLatin1String("Even"), QSerialPort::EvenParity);
    ui.QCB_Serial_parity->addItem(QLatin1String("Odd"), QSerialPort::OddParity);
    ui.QCB_Serial_parity->addItem(QLatin1String("Space"), QSerialPort::SpaceParity);
    ui.QCB_Serial_parity->addItem(QLatin1String("Mark"), QSerialPort::MarkParity);
    ui.QCB_Serial_parity->setCurrentIndex(ui.QCB_Serial_parity->findData(settings.pParity));

    ui.QCB_Serial_stopBits->clear();
    ui.QCB_Serial_stopBits->addItem(QLatin1String("1"), QSerialPort::OneStop);
    ui.QCB_Serial_stopBits->addItem(QLatin1String("1.5"), QSerialPort::OneAndHalfStop);
    ui.QCB_Serial_stopBits->addItem(QLatin1String("2"), QSerialPort::TwoStop);
    ui.QCB_Serial_stopBits->setCurrentIndex(ui.QCB_Serial_stopBits->findData(settings.pStopBits));


    ui.QCB_Serial_flowControl->clear();
    ui.QCB_Serial_flowControl->addItem(QLatin1String("None"), QSerialPort::NoFlowControl);
    ui.QCB_Serial_flowControl->addItem(QLatin1String("RTS/CTS"), QSerialPort::HardwareControl);
    ui.QCB_Serial_flowControl->addItem(QLatin1String("XON/XOFF"), QSerialPort::SoftwareControl);
    ui.QCB_Serial_flowControl->setCurrentIndex(ui.QCB_Serial_flowControl->findData(settings.pFlowControl));


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

    ui.le_cmd_start->setText(settings.CmdStart);
    ui.le_cmd_stop->setText(settings.CmdStop);
    ui.le_cmd_init->setText(settings.CmdInit);
    ui.le_cmd_reset->setText(settings.CmdReset);
    ui.le_cmd_center->setText(settings.CmdCenter);
    ui.le_cmd_zero->setText(settings.CmdZero);

    ui.spb_BeforeInit->setValue(settings.DelayInit);
    ui.spb_BeforeStart->setValue(settings.DelayStart);
    ui.spb_AfterStart->setValue(settings.DelaySeq);

    ui.cb_Endian->setChecked(settings.BigEndian);

#ifdef OPENTRACK_API
    ui.spb_Fps->setValue(settings.FPSArduino);
    connect(ui.spb_Fps, SIGNAL(valueChanged (  int  )),   this,SLOT(set_Fps(int)));
#endif

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

    connect(ui.le_cmd_start, SIGNAL(textEdited (QString )),     this,SLOT(set_cmd_start(QString)));
    connect(ui.le_cmd_stop, SIGNAL(textEdited (  QString  )),   this,SLOT(set_cmd_stop(QString)));
    connect(ui.le_cmd_init, SIGNAL(textChanged (  QString  )),  this,SLOT(set_cmd_init(QString)));
    connect(ui.le_cmd_reset, SIGNAL(textChanged (  QString  )), this,SLOT(set_cmd_reset(QString)));
    connect(ui.le_cmd_center, SIGNAL(textChanged (  QString  )),this,SLOT(set_cmd_center(QString)));
    connect(ui.le_cmd_zero, SIGNAL(textChanged (  QString  )),this,SLOT(set_cmd_zero(QString)));

    connect(ui.spb_BeforeInit, SIGNAL(valueChanged (  int  )),   this,SLOT(set_DelayInit(int)));
    connect(ui.spb_BeforeStart, SIGNAL(valueChanged (  int  )),  this,SLOT(set_DelayStart(int)));
    connect(ui.spb_AfterStart, SIGNAL(valueChanged (  int  )),   this,SLOT(set_DelaySeq(int)));

    connect( ui.cb_Endian,SIGNAL(toggled(bool)),			 this,SLOT(set_endian(bool)) );


    connect(ui.QCB_Serial_baudRate,  SIGNAL(currentIndexChanged(int)), this,SLOT(set_mod_baud(int)) );
    connect(ui.QCB_Serial_dataBits,  SIGNAL(currentIndexChanged(int)), this,SLOT(set_mod_dataBits(int)) );
    connect(ui.QCB_Serial_parity,  SIGNAL(currentIndexChanged(int)), this,SLOT(set_mod_parity(int)) );
    connect(ui.QCB_Serial_stopBits,  SIGNAL(currentIndexChanged(int)), this,SLOT(set_mod_stopBits(int)) );
    connect(ui.QCB_Serial_flowControl,  SIGNAL(currentIndexChanged(int)), this,SLOT(set_mod_flowControl(int)) );

    connect(ui.btnReset, SIGNAL(clicked()), this, SLOT(doReset()));
    connect(ui.btnCenter, SIGNAL(clicked()), this, SLOT(doCenter()));
    connect(ui.btnZero, SIGNAL(clicked()), this, SLOT(doZero()));
    connect(ui.btnSend, SIGNAL(clicked()), this, SLOT(doSend()));

    connect(ui.btn_icone, SIGNAL(clicked()), this, SLOT(doSerialInfo()));

    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));

    connect(ui.lineSend,SIGNAL(keyPressEvent),this,SLOT(on_lineSend_returnPressed()) );

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
#ifdef OPENTRACK_API
    if (theTracker) theTracker->center();
#else
    if (theTracker) theTracker->notifyCenter();
#endif
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
// Enter on lineSend for send to ARDUINO
//
void TrackerControls::on_lineSend_returnPressed()
{
    this->doSend();

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
    theTracker = static_cast<FTNoIR_Tracker*>(tracker);
    connect(theTracker, SIGNAL(sendMsgInfo(QByteArray)),this , SLOT(WriteMsgInfo(QByteArray)));

    if (isVisible() && settingsDirty) theTracker->applysettings(settings);

    ui.cbSerialPort->setEnabled(false);
    ui.btnZero->setEnabled(true);
    ui.btnCenter->setEnabled(true);
    ui.btnReset->setEnabled(true);
    ui.pteINFO->clear();
    ui.lab_vstatus->setText("HAT START");
    last_time.start();
    timer.start(250);

}


void TrackerControls::unRegisterTracker() {
    timer.stop();
    theTracker=NULL;
    ui.cbSerialPort->setEnabled(true);
    ui.btnZero->setEnabled(false);
    ui.btnCenter->setEnabled(false);
    ui.btnReset->setEnabled(false);
    ui.lab_vstatus->setText("HAT STOPPED");
    ui.lab_vtps->setText("");
}




////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
#ifdef OPENTRACK_API
extern "C" FTNOIR_TRACKER_BASE_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog( )
#else
#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")
FTNOIR_TRACKER_BASE_EXPORT ITrackerDialogPtr __stdcall GetTrackerDialog( )
#endif
{
    return new TrackerControls;
}

