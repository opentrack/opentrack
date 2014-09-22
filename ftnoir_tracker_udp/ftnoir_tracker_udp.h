#pragma once
#include "ui_ftnoir_ftnclientcontrols.h"
#include <QUdpSocket>
#include <QThread>
#include <cmath>
#include "facetracknoir/plugin-api.hpp"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> port;
    settings() :
        b(bundle("udp-tracker")),
        port(b, "port", 4242)
    {}
};

class FTNoIR_Tracker : public ITracker, public QThread
{
public:
	FTNoIR_Tracker();
    ~FTNoIR_Tracker();
    void StartTracker(QFrame *);
    void GetHeadPoseData(double *data);
    volatile bool should_quit;
protected:
	void run();												// qthread override run method
private:
    QUdpSocket inSocket;
    QHostAddress destIP;
    QHostAddress srcIP;
    double newHeadPose[6];
    QMutex mutex;
    settings s;
};

class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}
private:
	Ui::UICFTNClientControls ui;
    settings s;
private slots:
	void doOK();
	void doCancel();
};

class FTNoIR_TrackerDll : public Metadata
{
public:
	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);
};

