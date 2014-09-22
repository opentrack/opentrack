#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_ftnoir_hydra_clientcontrols.h"
#include "facetracknoir/plugin-support.h"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    settings() :
        b(bundle("tracker-hydra"))
    {}
};

class Hydra_Tracker : public ITracker
{
public:
	Hydra_Tracker();
    ~Hydra_Tracker();
    void StartTracker(QFrame *) override;
    void GetHeadPoseData(double *data) override;
    virtual int preferredHz() override { return 250; }
    volatile bool should_quit;
private:
    settings s;
    QMutex mutex;
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

