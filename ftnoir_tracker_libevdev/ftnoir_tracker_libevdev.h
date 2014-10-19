#pragma once
#include <cmath>
#include "libevdev/libevdev.h"
#include "facetracknoir/plugin-api.hpp"
#include "facetracknoir/options.h"
#include "./ui_ftnoir_libevdev.h"
#include <QThread>
#include <QMutex>
#include <QMutexLocker>
using namespace options;

struct settings {
    pbundle b;
    value<QString> device_name;
    settings() :
        b(bundle("libevdev-tracker")),
        device_name(b, "device-name", "")
    {}
};

class FTNoIR_Tracker : public ITracker, private QThread
{
public:
    FTNoIR_Tracker();
    ~FTNoIR_Tracker() override;
    void start_tracker(QFrame *);
    void data(double *data);
private:
    void run() override;
    struct libevdev* node;
    int fd;
    settings s;
    bool success;
    int a_min[6], a_max[6], values[6];
    QMutex mtx;
    volatile bool should_quit;
};

class TrackerControls: public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
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
    QString name() { return QString("libevdev joystick input"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};
