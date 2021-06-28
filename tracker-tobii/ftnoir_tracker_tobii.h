#pragma once
#include "ui_ftnoir_tracker_tobii_controls.h"
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QMessageBox>
#include <QSettings>
#include <QList>
#include <QFrame>
#include <QStringList>
#include <cmath>
#include "api/plugin-api.hpp"
#include "options/options.hpp"

#include "thread.hpp"

class tobii : public ITracker
{
public:
    ~tobii();
    module_status start_tracker(QFrame*) override;
    void data(double* data) override;
    virtual bool center() override;
private:
    tobii_thread t;
    tobii_head_pose_t center_pose;
};

class dialog_tobii: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_tobii();
    ~dialog_tobii() = default;
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
    Ui::UITobiiControls ui;
    tobii* tracker;
private slots:
    void doOK();
    void doCancel();
};

class tobiiDll : public Metadata
{
    Q_OBJECT

    QString name() { return tr("Tobii input"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};
