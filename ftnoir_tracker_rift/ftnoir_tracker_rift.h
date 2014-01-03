#pragma once
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_rift_clientcontrols.h"
#include <QMessageBox>
#include <QWaitCondition>
#include <cmath>
#include "facetracknoir/global-settings.h"
#include "OVR.h"
#include <memory>
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<bool> bEnableYaw, bEnablePitch, bEnableRoll, useYawSpring;
    value<double> constant_drift, persistence, deadzone;
    settings() :
        b(bundle("Rift")),
        bEnableYaw(b, "EnableYaw", true),
        bEnablePitch(b, "EnablePitch", true),
        bEnableRoll(b, "EnableRoll", true),
        useYawSpring(b, "yaw-spring", false),
        constant_drift(b, "constant-drift", 0.000005),
        persistence(b, "persistence", 0.99999),
        deadzone(b, "deadzone", 0.02)
    {}
};

class Rift_Tracker : public ITracker
{
public:
	Rift_Tracker();
    virtual ~Rift_Tracker() virt_override;

    void StartTracker(QFrame *) virt_override;
    void GetHeadPoseData(double *data) virt_override;
    virtual int preferredHz() virt_override { return 250; }
    volatile bool should_quit;
protected:
	void run();												// qthread override run method

private:
	static bool isInitialised;
	OVR::DeviceManager* pManager;
	OVR::SensorDevice* pSensor;
	OVR::SensorFusion* pSFusion;
    settings s;
    double old_yaw;
};

class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
	explicit TrackerControls();

    void registerTracker(ITracker *) {}
	void unRegisterTracker() {}

private:
	Ui::UIRiftControls ui;
    settings s;
private slots:
	void doOK();
	void doCancel();
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

