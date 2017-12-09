#pragma once
#include "ui_dialog.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
#include <OVR_CAPI.h>
#include <cmath>
#include <QMessageBox>
#include <QWaitCondition>
using namespace options;

struct settings : opts {
    value<bool> useYawSpring;
    value<double> constant_drift, persistence, deadzone;
    settings() :
        opts("Rift-140"),
        useYawSpring(b, "yaw-spring", false),
        constant_drift(b, "constant-drift", 0.000005),
        persistence(b, "persistence", 0.99999),
        deadzone(b, "deadzone", 0.02)
    {}
};

class rift_tracker_140 : public ITracker
{
public:
    rift_tracker_140();
    ~rift_tracker_140() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
private:
    double old_yaw;
    ovrSession hmd;
    ovrGraphicsLuid luid;
    settings s;
};

class dialog_rift_140: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_rift_140();

    void register_tracker(ITracker *) {}
    void unregister_tracker() {}

private:
    Ui::dialog_rift_140 ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class rift_140Dll : public Metadata
{
public:
    QString name() { return otr_tr("Oculus Rift runtime 1.4.0 -- HMD"); }
    QIcon icon() { return QIcon(":/images/rift_tiny.png"); }
};

