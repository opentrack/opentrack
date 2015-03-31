#ifndef FTNOIR_TRACKER_HAT_H
#define FTNOIR_TRACKER_HAT_H

#ifdef OPENTRACK_API
#   include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#   include "facetracknoir/plugin-support.h"
#else
#	include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
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

#define VER_FILEVERSION_STR         "Version 2.1.0\0"

class FTNoIR_Tracker : public QObject, public ITracker 
{ 
  Q_OBJECT
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

#ifdef OPENTRACK_API
    void StartTracker(QFrame*);
    void GetHeadPoseData(double *data);
    int preferredHz();
    void center();
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
#ifdef OPENTRACK_API
    int iFpsArduino;
#endif
    int CptError;


};


//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
class TrackerDll :
#if defined(OPENTRACK_API)
        public Metadata
#else
        public ITrackerDll
#endif
{
public:
	TrackerDll();
	~TrackerDll();

#ifndef OPENTRACK_API
	void Initialize();
#endif

	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);

private:
	QString trackerFullName;									// Trackers' name and description
	QString trackerShortName;
	QString trackerDescription;
};


#endif // FTNOIR_TRACKER_HAT_H
