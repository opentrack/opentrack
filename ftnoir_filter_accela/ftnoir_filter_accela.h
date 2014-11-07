#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "opentrack/plugin-api.hpp"
#include <atomic>
#include <QMutex>
#include <QTimer>

#include "opentrack/options.hpp"
using namespace options;

struct settings {
    pbundle b;
    value<double> rot_deadzone, trans_deadzone;
    value<int> rot_plus, rot_minus, trans_smoothing;
    settings() :
        b(bundle("Accela")),
        rot_deadzone(b, "rotation-deadband", 0),
        trans_deadzone(b, "translation-deadband", 0),
        rot_plus(b, "rotation-increase", 30),
        rot_minus(b, "rotation-decrease", 25),
        trans_smoothing(b, "translation-smoothing", 50)
    {}
};

struct state_display
{
    double y, p, r;
    state_display() : y(0), p(0), r(0) {};
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double* target_camera_position, double *new_camera_position);
    state_display state;
    static constexpr double Hz = 3./1000;
private:
    // moving average history amount
    static constexpr double fast_alpha_seconds = 0.7;
    // max degrees considered "slow" after moving average
    static constexpr double max_slow_delta = 0.9;
    settings s;
    bool first_run;
    double last_output[6];
    double fast_state[3];
};

class FilterControls: public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void register_filter(IFilter* filter);
    void unregister_filter();
private:
    Ui::AccelaUICFilterControls ui;
    void discard();
    void save();
    FTNoIR_Filter* accela_filter;
    settings s;
    QTimer t;
private slots:
    void doOK();
    void doCancel();
    void timer_fired();
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Accela"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
