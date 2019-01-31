#pragma once
#include "ui_test.h"
#include "compat/macros.hpp"
#include "api/plugin-api.hpp"


class test_dialog : public ITrackerDialog
{
    Q_OBJECT

    Ui::test_ui ui;
public:
    test_dialog();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private slots:
    void doOK();
    void doCancel();
};

class test_metadata : public Metadata
{
    Q_OBJECT

    QString name() { return tr("Kinect Face"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};

