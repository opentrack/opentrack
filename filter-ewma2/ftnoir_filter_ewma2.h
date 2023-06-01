#pragma once

#include "api/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include "options/options.hpp"
#include "compat/timer.hpp"
using namespace options;

struct settings : opts {
    // these are sadly sliders for now due to int/double mismatch -sh
    value<slider_value> kMinSmoothing, kMaxSmoothing, kSmoothingScaleCurve;
    settings() :
        opts("ewma-filter"),
        kMinSmoothing(b, "min-smoothing", { .02, .01, 1 }),
        kMaxSmoothing(b, "max-smoothing", { .7, .01, 1 }),
        kSmoothingScaleCurve(b, "smoothing-scale-curve", { .8, .1, 5 })
    {}
};

class ewma : public IFilter
{
public:
    ewma();
    void filter(const double *input, double *output) override;
    void center() override { first_run = true; }
    module_status initialize() override { return status_ok(); }
private:
    // Deltas are smoothed over the last 1/60sec.
    static constexpr double delta_RC = 1./60;
    // Noise is smoothed over the last 60sec.
    static constexpr double noise_RC_max = 60.0;
    double noise_RC = 0.0;
    double last_delta[6];
    double last_noise[6];
    double last_output[6];
    Timer timer;
    settings s;
    bool first_run = true;
};

class dialog_ewma: public IFilterDialog
{
    Q_OBJECT
public:
    dialog_ewma();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}

private:
    Ui::UICdialog_ewma ui;
    settings s;

private slots:
    void doOK();
    void doCancel();
};

class ewmaDll : public Metadata
{
    Q_OBJECT

    QString name() { return tr("EWMA"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
