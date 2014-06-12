#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_hydra_clientcontrols.h"
#include <QMessageBox>
#include <QWaitCondition>
#include <math.h>
#include "facetracknoir/global-settings.h"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<bool> bEnableRoll, bEnablePitch, bEnableYaw, bEnableX, bEnableY, bEnableZ;
    settings() :
        b(bundle("tracker-hydra")),
        bEnableRoll(b, "enable-rz", true),
        bEnablePitch(b, "enable-ry", true),
        bEnableYaw(b, "enable-rx", true),
        bEnableX(b, "enable-tx", true),
        bEnableY(b, "enable-ty", true),
        bEnableZ(b, "enable-tz", true)
    {}
};

class Hydra_Tracker : public ITracker
{
public:
	Hydra_Tracker();
    ~Hydra_Tracker();
    void StartTracker(QFrame *) virt_override;
    void GetHeadPoseData(double *data) virt_override;
    volatile bool should_quit;
protected:
	void run();												// qthread override run method
private:
    settings s;
	bool isCalibrated;
    double newHeadPose[6];								// Structure with new headpose
    QMutex mutex;
    virtual int preferredHz() virt_override { return 250; }
};

class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
	explicit TrackerControls();
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}
private:
    settings s;
	Ui::UIHydraControls ui;
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

