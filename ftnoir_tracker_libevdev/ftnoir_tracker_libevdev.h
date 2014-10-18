#pragma once
#include <cmath>
#include "libevdev/libevdev.h"
#include "facetracknoir/plugin-api.hpp"
#include "facetracknoir/options.h"
#include "./ui_ftnoir_libevdev.h"
using namespace options;

struct settings {
    pbundle b;
    value<QString> device_name;
    settings() :
        b(bundle("libevdev-tracker")),
        device_name(b, "device-name", "")
    {}
};

class FTNoIR_Tracker : public ITracker
{
public:
	FTNoIR_Tracker();
    ~FTNoIR_Tracker() override;
    void StartTracker(QFrame *);
    void GetHeadPoseData(double *data);
private:
    struct libevdev* node;
    int fd;
    settings s;
    bool success;
    int a_min[6], a_max[6];
};

class TrackerControls: public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
	TrackerControls();
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}
private:
	Ui::ui_libevdev_tracker_dialog ui;
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
