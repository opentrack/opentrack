#pragma once

#include "ui_ftnoir_hydra_clientcontrols.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    settings() :
        opts("tracker-hydra")
    {}
};

class Hydra_Tracker : public ITracker
{
public:
    Hydra_Tracker();
    ~Hydra_Tracker();
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;

private:
    settings s;
    QMutex mutex;
};

class dialog_hydra: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_hydra();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
private:
    settings s;
    Ui::UIHydraControls ui;
private slots:
    void doOK();
    void doCancel();
};

class hydraDll : public Metadata
{
public:
    QString name() { return QString("Razer Hydra -- inertial device"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};

