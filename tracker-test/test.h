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
    static const double incr[6];
    double last_x[6];
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
public:
    QString name() { return otr_tr("Testing - sine wave"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};

