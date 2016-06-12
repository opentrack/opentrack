#pragma once
#include "ui_test.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack-compat/timer.hpp"

class FTNoIR_Tracker : public ITracker
{
public:
    FTNoIR_Tracker();
    ~FTNoIR_Tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;

private:
    static constexpr double pi = 3.14159265358979323846;
    static constexpr double r2d = 180 / pi;
    static constexpr double d2r = pi / 180;

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

