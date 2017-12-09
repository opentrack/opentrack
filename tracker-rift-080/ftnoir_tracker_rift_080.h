#pragma once
#include "ui_ftnoir_rift_clientcontrols_080.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
#include <OVR.h>
#include <cmath>
#include <QMessageBox>
#include <QWaitCondition>
using namespace options;

struct settings : opts {
    value<bool> useYawSpring;
    value<double> constant_drift, persistence, deadzone;
    settings() :
        opts("Rift-080"),
        useYawSpring(b, "yaw-spring", false),
        constant_drift(b, "constant-drift", 0.000005),
        persistence(b, "persistence", 0.99999),
        deadzone(b, "deadzone", 0.02)
    {}
};

class rift_tracker_080 : public ITracker
{
public:
    rift_tracker_080();
    ~rift_tracker_080() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
private:
    double old_yaw;
    ovrSession hmd;
    settings s;
};

class dialog_rift_080: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_rift_080();

    void register_tracker(ITracker *) {}
    void unregister_tracker() {}

private:
    Ui::dialog_rift_080 ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class rift_080Dll : public Metadata
{
public:
    QString name() { return otr_tr("Oculus Rift runtime 0.8.0 -- HMD"); }
    QIcon icon() { return QIcon(":/images/rift_tiny.png"); }
};

