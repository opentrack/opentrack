#ifndef FTNOIR_TRACKER_HAT_H
#define FTNOIR_TRACKER_HAT_H

#include "facetracknoir/global-settings.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_tracker_hat_settings.h"
#include "ftnoir_arduino_type.h"

#include <QtSerialPort/QSerialPort>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include <QMutex>
#include <QMutexLocker>
#include <cmath>

class FTNoIR_Tracker : public ITracker, QThread
{ 

public:
	FTNoIR_Tracker();
    virtual ~FTNoIR_Tracker() virt_override;

    virtual void StartTracker( QFrame* frame ) virt_override;
    virtual bool GiveHeadPoseData(double *data) virt_override;

	void applysettings(const TrackerSettings& settings);
	void notifyCenter();
    void center();
	void reset();
	void sendcmd(QString*  cmd);
	void get_info(QString* info , int* tps );

protected:
	void run();												// qthread override run method

private:
    TArduinoData ArduinoData, HAT ;                              // Trame from Arduino
    QByteArray datagram;
    QSerialPort* SerialPort;
    volatile bool stop;
    QMutex lock;
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


    int iRollAxis;
    int iPitchAxis;
    int iYawAxis;
    int iXAxis;
    int iYAxis;
    int iZAxis;

};

class FTNoIR_TrackerDll : public Metadata
{
public:
	FTNoIR_TrackerDll();
	~FTNoIR_TrackerDll();

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
