#pragma once

#include "thread.hpp"
#include "opentrack/plugin-support.hpp"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_arduino_type.h"

#include <QObject>
#include <QPalette>
#include <QtGui>
#include <QByteArray>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QSettings>

#define VER_FILEVERSION_STR         "Version 2.1.1"

class hatire : public QObject, public ITracker
{
    Q_OBJECT

public:
    hatire();
    ~hatire();

    void start_tracker(QFrame*);
    void data(double *data);
    //void center();
    void applysettings(const TrackerSettings& settings);
    //bool notifyZeroed();
    void reset();
    void get_info( int *tps );
    void serial_info();
    void send_serial_command(const QByteArray& x);

private:
    TArduinoData ArduinoData, HAT;
    QByteArray Begin;
    QByteArray End;

    hatire_thread t;
    thread_settings ts;

    // XXX move to settings api -sh 20160410
    TrackerSettings settings;

    int frame_cnt;

    bool bEnableRoll;
    bool bEnablePitch;
    bool bEnableYaw;
    bool bEnableX;
    bool bEnableY;
    bool bEnableZ;

    bool bInvertRoll;
    bool bInvertPitch;
    bool bInvertYaw;
    bool bInvertX;
    bool bInvertY;
    bool bInvertZ;

    int iRollAxis;
    int iPitchAxis;
    int iYawAxis;
    int iXAxis;
    int iYAxis;
    int iZAxis;

    volatile int CptError;
};

class TrackerDll : public Metadata
{
    QString name() { return QString("Hatire Arduino"); }
    QIcon icon() { return QIcon(":/images/hat.png"); }
};
