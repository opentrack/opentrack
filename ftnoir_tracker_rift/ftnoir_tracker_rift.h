#pragma once
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_rift_clientcontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QWaitCondition>
#include <cmath>
#include "facetracknoir/global-settings.h"
#include "OVR.h"
#include <memory>
class Rift_Tracker : public ITracker
{
public:
	Rift_Tracker();
    virtual ~Rift_Tracker() virt_override;

    void StartTracker(QFrame *) virt_override;
    bool GiveHeadPoseData(double *data) virt_override;
    void loadSettings();
    virtual int preferredHz() virt_override { return 250; }
    volatile bool should_quit;
protected:
	void run();												// qthread override run method

private:
	static bool isInitialised;
	OVR::DeviceManager* pManager;
	OVR::SensorDevice* pSensor;
	OVR::SensorFusion* pSFusion;
	bool bEnableRoll;
	bool bEnablePitch;
	bool bEnableYaw;
#if 1
	bool bEnableX;
	bool bEnableY;
	bool bEnableZ;
#endif
    bool useYawSpring;
    double old_yaw, constant_drift, persistence, deadzone, old_x, old_y, old_z, vx, vy, vz;
};

// Widget that has controls for FTNoIR protocol client-settings.
class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:

	explicit TrackerControls();
    ~TrackerControls();
    void showEvent (QShowEvent *);

    void Initialize(QWidget *parent);
    void registerTracker(ITracker *) {}
	void unRegisterTracker() {}

private:
	Ui::UIRiftControls ui;
	void loadSettings();
	void save();

	/** helper **/
	bool settingsDirty;

private slots:
	void doOK();
	void doCancel();
	void settingChanged() { settingsDirty = true; }
	void settingChanged(int) { settingsDirty = true; }
};

//*******************************************************************************************************
// FaceTrackNoIR Tracker DLL. Functions used to get general info on the Tracker
//*******************************************************************************************************
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

