/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#include "ftnoir_tracker_hat.h"
#include "ftnoir_tracker_hat_dialog.h"

#include <QScrollBar>

dialog_hatire::dialog_hatire() : theTracker(nullptr), timer(this)
{
    ui.setupUi(this);

    ui.label_version->setText(tr("Version %1").arg("2.1.1"));

    // make SerialPort list
    ui.cbSerialPort->clear();
    for (const QSerialPortInfo& port_info : QSerialPortInfo::availablePorts() ) {
        ui.cbSerialPort->addItem(port_info.portName());
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
    ui.QCB_Serial_stopBits->addItem(QLatin1String("1"), QSerialPort::OneStop);
    ui.QCB_Serial_stopBits->addItem(QLatin1String("1.5"), QSerialPort::OneAndHalfStop);
    ui.QCB_Serial_stopBits->addItem(QLatin1String("2"), QSerialPort::TwoStop);

    ui.QCB_Serial_flowControl->clear();
    ui.QCB_Serial_flowControl->addItem(QLatin1String("None"), QSerialPort::NoFlowControl);
    ui.QCB_Serial_flowControl->addItem(QLatin1String("RTS/CTS"), QSerialPort::HardwareControl);
    ui.QCB_Serial_flowControl->addItem(QLatin1String("XON/XOFF"), QSerialPort::SoftwareControl);

    tie_setting(s.EnableYaw, ui.chkEnableYaw);
    tie_setting(s.EnablePitch, ui.chkEnablePitch);
    tie_setting(s.EnableRoll, ui.chkEnableRoll);
    tie_setting(s.EnableX, ui.chkEnableX);
    tie_setting(s.EnableY, ui.chkEnableY);
    tie_setting(s.EnableZ, ui.chkEnableZ);

    tie_setting(s.InvertYaw, ui.chkInvertYaw);
    tie_setting(s.InvertPitch, ui.chkInvertPitch);
    tie_setting(s.InvertRoll, ui.chkInvertRoll);
    tie_setting(s.InvertX, ui.chkInvertX);
    tie_setting(s.InvertY, ui.chkInvertY);
    tie_setting(s.InvertZ, ui.chkInvertZ);

    tie_setting(s.EnableLogging, ui.chkEnableLogging);

    tie_setting(s.YawAxis, ui.cb_yaw);
    tie_setting(s.PitchAxis, ui.cb_pitch);
    tie_setting(s.RollAxis, ui.cb_roll);
    tie_setting(s.XAxis, ui.cb_x);
    tie_setting(s.YAxis, ui.cb_y);
    tie_setting(s.ZAxis, ui.cb_z);

    tie_setting(s.CmdStart, ui.le_cmd_start);
    tie_setting(s.CmdStop, ui.le_cmd_stop);
    tie_setting(s.CmdInit, ui.le_cmd_init);
    tie_setting(s.CmdReset, ui.le_cmd_reset);
    tie_setting(s.CmdCenter, ui.le_cmd_center);
    tie_setting(s.CmdZero, ui.le_cmd_zero);

    tie_setting(s.DelayInit, ui.spb_BeforeInit);
    tie_setting(s.DelayStart, ui.spb_BeforeStart);
    tie_setting(s.DelaySeq, ui.spb_AfterStart);

    tie_setting(s.BigEndian, ui.cb_Endian);
    tie_setting(s.pDTR, ui.QCB_Serial_dtr);

    tie_setting(s.pBaudRate, ui.QCB_Serial_baudRate);
    tie_setting(s.pDataBits, ui.QCB_Serial_dataBits);
    tie_setting(s.pFlowControl, ui.QCB_Serial_flowControl);
    tie_setting(s.pParity, ui.QCB_Serial_parity);
    tie_setting(s.pStopBits, ui.QCB_Serial_stopBits);

    tie_setting(s.QSerialPortName, ui.cbSerialPort);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    connect(ui.btnReset, SIGNAL(clicked()), this, SLOT(doReset()));
    //connect(ui.btnCenter, SIGNAL(clicked()), this, SLOT(doCenter()));
    connect(ui.btnZero, SIGNAL(clicked()), this, SLOT(doZero()));
    connect(ui.btnSend, SIGNAL(clicked()), this, SLOT(doSend()));

    connect(ui.btn_icone, SIGNAL(clicked()), this, SLOT(doSerialInfo()));

    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));

    // can't connect slot, keyPressEvent takes QKeyPressEvent as argument
    //connect(ui.lineSend,SIGNAL(keyPressEvent),this,SLOT(on_lineSend_returnPressed()) );
}

dialog_hatire::~dialog_hatire() = default;

void dialog_hatire::Initialize(QWidget *parent)
{
    QPoint offsetpos(100, 100);
    if (parent)
        move(parent->pos() + offsetpos);
    show();
}

//
// Zero asked to ARDUINO
//
void dialog_hatire::doZero() {
    //if (theTracker) theTracker->notifyZeroed();
}

//
// Reset asked to ARDUINO
//
void dialog_hatire::doReset() {
    if (theTracker) theTracker->reset();
}


//
// Serial Info debug
//
void dialog_hatire::doSerialInfo() {
    if (theTracker) theTracker->serial_info();
}

//
// Send command to ARDUINO
//
void dialog_hatire::doSend() {
    if (theTracker) {
        if (!ui.lineSend->text().isEmpty()) {
            theTracker->send_serial_command(ui.lineSend->text().toLatin1());
        }
    }
}

//
// Enter on lineSend for send to ARDUINO
//
void dialog_hatire::on_lineSend_returnPressed()
{
    doSend();
}

//
// Display FPS  of Arduino.
//
void dialog_hatire::poll_tracker_info()
{
    if (theTracker)
    {
        int frame_cnt;

        theTracker->get_info(&frame_cnt);
        ui.lab_vtps->setText(QString::number(frame_cnt*(1000/last_time.elapsed_ms())));
        last_time.start();
    }
}

void dialog_hatire::WriteMsgInfo(const QByteArray &MsgInfo)
{
    QApplication::beep();
    ui.pteINFO->moveCursor(QTextCursor::End);
    ui.pteINFO->insertPlainText(QString(MsgInfo));
    QScrollBar *bar = ui.pteINFO->verticalScrollBar();
    bar->setValue(bar->maximum());
}

void dialog_hatire::doOK()
{
    s.b->save();
    close();
}

void dialog_hatire::doCancel()
{
    close();
}

void dialog_hatire::register_tracker(ITracker *tracker)
{
    theTracker = static_cast<hatire*>(tracker);
    connect(&theTracker->t, SIGNAL(serial_debug_info(QByteArray)), this, SLOT(WriteMsgInfo(QByteArray)));

    ui.cbSerialPort->setEnabled(false);
    ui.btnZero->setEnabled(true);
    //ui.btnCenter->setEnabled(true);
    ui.btnReset->setEnabled(true);
    ui.pteINFO->clear();
    ui.lab_vstatus->setText(tr("HAT START"));
    last_time.start();
    timer.start(250);
}

void dialog_hatire::unregister_tracker()
{
    timer.stop();
    theTracker = nullptr;
    ui.cbSerialPort->setEnabled(true);
    ui.btnZero->setEnabled(false);
    //ui.btnCenter->setEnabled(false);
    ui.btnReset->setEnabled(false);
    ui.lab_vstatus->setText(tr("HAT STOPPED"));
    ui.lab_vtps->setText("");
}
