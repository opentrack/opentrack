#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "opentrack/plugin-api.hpp"
#include <atomic>
#include <QMutex>
#include <QTimer>

#define ACCELA_SMOOTHING_ROTATION 60.0
#define ACCELA_SMOOTHING_TRANSLATION 40.0
#define ACCELA_fast_ALPHA 100.0
#define ACCELA_THIRD_ORDER_ALPHA 180.0

#include "opentrack/options.hpp"
using namespace options;

struct settings {
    pbundle b;
    value<double> rotation_alpha,
                  translation_alpha,
                  rot_deadzone,
                  trans_deadzone,
                  expt;
    value<int> fast_alpha;
    settings() :
        b(bundle("Accela")),
        rotation_alpha(b, "rotation-alpha", ACCELA_SMOOTHING_ROTATION),
        translation_alpha(b, "translation-alpha", ACCELA_SMOOTHING_TRANSLATION),
        rot_deadzone(b, "rotation-deadband", 0),
        trans_deadzone(b, "translation-deadband", 0),
        expt(b, "exponent", 2),
        fast_alpha(b, "fast-alpha", 0)
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
private:
    // hardcoded distance between filter() calls
    static constexpr double Hz = 3./1000;
    // moving average history
    static constexpr double fast_alpha_seconds = 0.2;
    // max degrees considered "slow" after alpha
    static constexpr double max_slow_delta = 0.75;
    // if set to zero, never decreases response
    static constexpr double damping = 0.75;
    settings s;
    bool first_run;
    double last_output[6];
    double fast_state[6];
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
