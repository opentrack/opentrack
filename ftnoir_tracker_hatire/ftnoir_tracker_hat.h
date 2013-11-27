#ifndef FTNOIR_TRACKER_HAT_H
#define FTNOIR_TRACKER_HAT_H

#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_arduino_type.h"

#include <QExtSerialPort\qextserialport.h>
#include <QExtSerialPort\qextserialenumerator.h>
#include <QThread>
#include <QReadWriteLock>
#include <QTimer>
#include <QSettings>
#include "Windows.h"
#include "math.h"

class QextSerialPort;
class QExtSerialEnumerator;

class FTNoIR_Tracker : public ITracker, QThread
{ 

public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

    void Initialize( QFrame *videoframe );
    void StartTracker( HWND parent_window );
    void StopTracker( bool exit );
	bool GiveHeadPoseData(THeadPoseData *data);

	void applysettings(const TrackerSettings& settings);
	void notifyCenter();
    void center();
	void reset();
	void sendcmd(QString*  cmd);
	void get_info(QString* info , int* tps );

protected:
	void run();												// qthread override run method


private:
	// Handles to neatly terminate thread...
	HANDLE m_StopThread;
	HANDLE m_WaitThread;

    TArduinoData ArduinoData, HAT ;                              // Trame from Arduino
    QByteArray datagram;
 	QextSerialPort *SerialPort;
	QReadWriteLock rwlock;
	QList<QString>* ListErrInf ;
    int waitTimeout;
	QString sSerialPortName;									// Port serial name
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

};


//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
class FTNoIR_TrackerDll : public ITrackerDll
{
public:
	FTNoIR_TrackerDll();
	~FTNoIR_TrackerDll();

    void Initialize();

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