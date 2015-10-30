#ifndef FTNOIR_TRACKER_HAT_H
#define FTNOIR_TRACKER_HAT_H

#ifdef OPENTRACK_API
#   include "opentrack/plugin-support.hpp"
#else
#   include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#endif
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_arduino_type.h"
#include <QObject>
#include <QPalette>
#include <QtGui>
#include <QByteArray>
#include <QMessageBox>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QSettings>

#define VER_FILEVERSION_STR         "Version 2.1.1\0"

class FTNoIR_Tracker : public QObject, public ITracker 
{
    Q_OBJECT
public:
    FTNoIR_Tracker();
    ~FTNoIR_Tracker();

#ifdef OPENTRACK_API
    void start_tracker(QFrame*);
    void data(double *data);
    int preferredHz(); // unused
    void center(); // unused
#else
    void Initialize( QFrame *videoframe );
    void StartTracker(HWND parent_window);
    void StopTracker(bool exit);
    bool GiveHeadPoseData(THeadPoseData *data);
    void notifyCenter();
#endif
    void applysettings(const TrackerSettings& settings);
    bool notifyZeroed();
    void reset();
    void SerialInfo();
    void sendcmd(const QByteArray &cmd);
    void get_info( int *tps );

private Q_SLOTS:
    void SerialRead();
    void Log(QString message);

signals:
    void sendMsgInfo(const QByteArray &MsgInfo);


private:
    QSerialPort *ComPort;
    TArduinoData ArduinoData, HAT ;                              // Trame from Arduino
    QByteArray dataRead;
    QByteArray dataToSend;
    QByteArray Begin;
    QByteArray End;
    QMutex mutex;
    int frame_cnt;
    bool new_frame;

    TrackerSettings settings;

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
    bool bEnableLogging;

    int iRollAxe;
    int iPitchAxe;
    int iYawAxe;
    int iXAxe;
    int iYAxe;
    int iZAxe;

    QByteArray  sCmdStart;
    QByteArray  sCmdStop;
    QByteArray  sCmdInit;
    QByteArray  sCmdReset;
    QByteArray  sCmdCenter;
    QByteArray  sCmdZero;

    int iDelayInit;
    int iDelayStart;
    int iDelaySeq;

    bool bBigEndian;

    QString sSerialPortName;
    QSerialPort::BaudRate iBaudRate;
    QSerialPort::DataBits iDataBits;
    QSerialPort::Parity iParity;
    QSerialPort::StopBits iStopBits;
    QSerialPort::FlowControl iFlowControl;

    QFile flDiagnostics;
#ifdef OPENTRACK_API
    int iFpsArduino;
#endif
    int CptError;


};


//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
#if defined(OPENTRACK_API)
class TrackerDll : public Metadata
{
    QString name() { return QString("Hatire Arduino"); }
    QIcon icon() { return QIcon(":/images/hat.png"); }
};
#else
class TrackerDll :
public Metadata
public ITrackerDll
{
public:
    TrackerDll();
    ~TrackerDll();

    void Initialize();

    QString name();
    QIcon icon();
    void getFullName(QString *strToBeFilled);
    void getShortName(QString *strToBeFilled);
    void getDescription(QString *strToBeFilled);
    void getIcon(QIcon *icon);

private:
    QString trackerFullName;                                                                    // Trackers' name and description
    QString trackerShortName;
    QString trackerDescription;
};
#endif

#endif // FTNOIR_TRACKER_HAT_H
