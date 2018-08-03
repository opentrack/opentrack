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
    settings() : opts("Rift-140")
    {}
};

class rift_tracker_140 : public ITracker
{
public:
    rift_tracker_140();
    ~rift_tracker_140() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
    bool center() override;

private:
    ovrSession hmd = nullptr;
    ovrGraphicsLuid luid {};
    settings s;
};

class dialog_rift_140: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_rift_140();

    void register_tracker(ITracker*) override {}
    void unregister_tracker() override {}

private:
    Ui::dialog_rift_140 ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class rift_140Dll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Oculus Rift runtime 1.4.0 -- HMD"); }
    QIcon icon() override { return QIcon(":/images/rift_tiny.png"); }
};

