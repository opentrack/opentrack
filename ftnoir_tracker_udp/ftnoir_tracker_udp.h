#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_ftnclientcontrols.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include <QMutex>
#include <QWaitCondition>
#include <math.h>
#include "facetracknoir/plugin-support.h"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> port;
    value<bool> enable_roll, enable_pitch, enable_yaw,
                enable_x, enable_y, enable_z;
    settings() :
        b(bundle("udp-tracker")),
        port(b, "port", 4242),
        enable_roll(b, "enable-roll", true),
        enable_pitch(b, "enable-pitch", true),
        enable_yaw(b, "enable-yaw", true),
        enable_x(b, "enable-x", true),
        enable_y(b, "enable-y", true),
        enable_z(b, "enable-y", true)
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

