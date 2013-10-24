#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_ftnclientcontrols.h"
#include <QThread>
#include <QUdpSocket>
#include <QMessageBox>
#include <QSettings>
#include <QMutex>
#include <QWaitCondition>
#include <math.h>
#include "facetracknoir/global-settings.h"

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
	~FTNoIR_Tracker();

    void StartTracker(QFrame *);
    bool GiveHeadPoseData(double *data);
	void loadSettings();
private:
	bool bEnableRoll;
	bool bEnablePitch;
	bool bEnableYaw;
	bool bEnableX;
	bool bEnableY;
	bool bEnableZ;
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
	Ui::UICFTNClientControls ui;
	void loadSettings();
	void save();
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

