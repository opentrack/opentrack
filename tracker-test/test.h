#pragma once
#include "ui_test.h"
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"

#include <cmath>

class FTNoIR_Tracker : public ITracker
{
public:
    FTNoIR_Tracker();
    ~FTNoIR_Tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;

private:
    static constexpr double r2d = 180 / M_PI;
    static constexpr double d2r = M_PI / 180;

    static const double incr[6];
    double last_x[6];
    Timer t;
};

class TrackerControls: public ITrackerDialog
{
    Q_OBJECT

    Ui::test_ui ui;
public:
    TrackerControls();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_TrackerDll : public Metadata
{
public:
    QString name() { return QString("Testing - sine wave"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};

