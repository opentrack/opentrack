#pragma once
#include "ui_ftnoir_rift_clientcontrols.h"
#include <QMessageBox>
#include <QWaitCondition>
#include <cmath>
#include "facetracknoir/plugin-api.hpp"
#include "OVR.h"
#include <memory>
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<bool> useYawSpring;
    value<double> constant_drift, persistence, deadzone;
    settings() :
        b(bundle("Rift")),
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
    ~Rift_Tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;
private:
    double old_yaw;
    ovrHmd hmd;
    settings s;
};

class TrackerControls: public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();

    void register_tracker(ITracker *) {}
    void unregister_tracker() {}

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
    QString name() { return QString("Oculus Rift -- HMD"); }
    QIcon icon() { return QIcon(":/images/rift_tiny.png"); }
};

