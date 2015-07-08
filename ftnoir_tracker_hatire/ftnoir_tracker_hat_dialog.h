#ifndef FTNOIR_TRACKER_HAT_DIALOG_H
#define FTNOIR_TRACKER_HAT_DIALOG_H

#ifdef OPENTRACK_API
#include "opentrack/plugin-api.hpp"
#else
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#endif
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_tracker_hat.h"
#include "ui_ftnoir_hatcontrols.h"
#include <QObject>
#include <QWidget>
#include <QTime>
#include <QTimer>
#include <QMessageBox>
#include <QMetaType>

// Widget that has controls for FTNoIR protocol client-settings.
#ifdef OPENTRACK_API
class TrackerControls: public ITrackerDialog
        #else
class TrackerControls: public QWidget, public ITrackerDialog
        #endif
{
    Q_OBJECT
public:
    explicit TrackerControls();
    virtual ~TrackerControls();
#ifdef OPENTRACK_API
    void Initialize(QWidget *parent) ; // unused
    void register_tracker(ITracker *tracker);
    void unregister_tracker();
#else
    void Initialize(QWidget *parent) ;
    void registerTracker(ITracker *tracker) ;
    void unRegisterTracker() ;
#endif

private:
    Ui::UIHATControls ui;
    FTNoIR_Tracker *theTracker;
    QTime last_time;

public  slots:
    void WriteMsgInfo(const QByteArray &MsgInfo);

protected slots:
    void set_mod_port(const QString & val)     {  settings.SerialPortName =val;   settings_changed(); }
    void set_ena_roll(bool val)  { settings.EnableRoll = val;    settings_changed(); }
    void set_ena_pitch(bool val) { settings.EnablePitch = val;   settings_changed(); }
    void set_ena_yaw(bool val)   { settings.EnableYaw = val;     settings_changed(); }
    void set_ena_x(bool val)     { settings.EnableX = val;       settings_changed(); }
    void set_ena_y(bool val)     { settings.EnableY = val;       settings_changed(); }
    void set_ena_z(bool val)     { settings.EnableZ = val;       settings_changed(); }

    void set_inv_roll(bool val)  { settings.InvertRoll = val;    settings_changed(); }
    void set_inv_pitch(bool val) { settings.InvertPitch = val;   settings_changed(); }
    void set_inv_yaw(bool val)   { settings.InvertYaw = val;     settings_changed(); }
    void set_inv_x(bool val)     { settings.InvertX = val;       settings_changed(); }
    void set_inv_y(bool val)     { settings.InvertY = val;       settings_changed(); }
    void set_inv_z(bool val)     { settings.InvertZ = val;       settings_changed(); }

    void set_diag_logging(bool val)     { settings.EnableLogging = val;       settings_changed(); }


    void set_rot_roll(int val)    { settings.RollAxe = val;    settings_changed(); }
    void set_rot_pitch(int val)   { settings.PitchAxe = val;   settings_changed(); }
    void set_rot_yaw(int val)     { settings.YawAxe = val;     settings_changed(); }
    void set_acc_x(int val)       { settings.XAxe = val;       settings_changed(); }
    void set_acc_y(int val)       { settings.YAxe = val;       settings_changed(); }
    void set_acc_z(int val)       { settings.ZAxe = val;       settings_changed(); }

    void set_cmd_start(const QString &val)  { settings.CmdStart = val;   settings_changed(); }
    void set_cmd_stop(const QString &val)   { settings.CmdStop = val;    settings_changed(); }
    void set_cmd_init(const QString &val)   { settings.CmdInit = val;    settings_changed(); }
    void set_cmd_reset(const QString &val)  { settings.CmdReset = val;   settings_changed(); }
    void set_cmd_center(const QString &val) { settings.CmdCenter = val;  settings_changed(); }
    void set_cmd_zero(const QString &val)   { settings.CmdZero = val;  settings_changed(); }

    void set_DelayInit(int val)        { settings.DelayInit = val;      settings_changed(); }
    void set_DelayStart(int val)       { settings.DelayStart = val;     settings_changed(); }
    void set_DelaySeq(int val)         { settings.DelaySeq = val;       settings_changed(); }

    void set_endian(bool val)     { settings.BigEndian = val;       settings_changed(); }
#ifdef OPENTRACK_API
    void set_Fps(int val)         { settings.FPSArduino = val;      settings_changed(); }
#endif

    void set_mod_baud(int val)        { settings.pBaudRate    = static_cast<QSerialPort::BaudRate>(ui.QCB_Serial_baudRate->itemData(val).toInt()) ;       settings_changed();   }
    void set_mod_dataBits(int val)    { settings.pDataBits    = static_cast<QSerialPort::DataBits>(ui.QCB_Serial_dataBits->itemData(val).toInt()) ;       settings_changed();   }
    void set_mod_parity(int val)      { settings.pParity      = static_cast<QSerialPort::Parity>(ui.QCB_Serial_parity->itemData(val).toInt()) ;           settings_changed();   }
    void set_mod_stopBits(int val)    { settings.pStopBits    = static_cast<QSerialPort::StopBits>(ui.QCB_Serial_stopBits->itemData(val).toInt());        settings_changed();   }
    void set_mod_flowControl(int val) { settings.pFlowControl = static_cast<QSerialPort::FlowControl>(ui.QCB_Serial_flowControl->itemData(val).toInt()) ; settings_changed();   }

    void doOK();
    void doCancel();
    void doSave();
    void doReset();
    void doCenter();
    void doZero();
    void doSend();
    void poll_tracker_info();
    void doSerialInfo();

protected:
    bool settingsDirty;
    void settings_changed();
    TrackerSettings settings;
    QTimer timer;

private slots:
    void on_lineSend_returnPressed();
};

#endif //FTNOIR_TRACKER_HAT_DIALOG_H
