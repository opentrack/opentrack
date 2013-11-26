#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_hydra_clientcontrols.h"
#include <QMessageBox>
#include <QSettings>
#include <QWaitCondition>
#include <math.h>
#include "facetracknoir/global-settings.h"
class Hydra_Tracker : public ITracker
{
public:
	Hydra_Tracker();
    ~Hydra_Tracker();

    void StartTracker(QFrame *) virt_override;
    bool GiveHeadPoseData(double *data) virt_override;
    void loadSettings();
    volatile bool should_quit;
protected:
	void run();												// qthread override run method

private:
	bool isCalibrated;

    double newHeadPose[6];								// Structure with new headpose
	bool bEnableRoll;
	bool bEnablePitch;
	bool bEnableYaw;

	bool bEnableX;
	bool bEnableY;
	bool bEnableZ;

    QMutex mutex;

    virtual int preferredHz() virt_override { return 250; }
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
	Ui::UIHydraControls ui;
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

