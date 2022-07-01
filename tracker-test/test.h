#pragma once
#include "ui_test.h"
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"

#include <cmath>

class test_tracker : public ITracker
{
public:
    test_tracker();
    ~test_tracker() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;

private:
    double last[6] {};
    Timer t;
};

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

    QString name() override { return tr("Test tracker"); }
    QIcon icon() override { return QIcon(":/images/opentrack.png"); }
};

