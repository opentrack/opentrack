#pragma once

#include "opentrack/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include "opentrack-compat/options.hpp"
#include "opentrack-compat/timer.hpp"
using namespace options;

struct settings : opts {
    // these are sadly sliders for now due to int/double mismatch -sh
    value<slider_value> kMinSmoothing, kMaxSmoothing, kSmoothingScaleCurve;
    settings() :
        opts("ewma-filter"),
        kMinSmoothing(b, "min-smoothing", slider_value(.02, .01, 1)),
        kMaxSmoothing(b, "max-smoothing", slider_value(.7, .01, 1)),
        kSmoothingScaleCurve(b, "smoothing-scale-curve", slider_value(.8, .1, 5))
    {}
};


class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double *input, double *output);
private:
    // Deltas are smoothed over the last 1/60sec.
    const double delta_RC = 1./60;
    // Noise is smoothed over the last 60sec.
    const double noise_RC = 60.0;
    double last_delta[6];
    double last_noise[6];
    double last_output[6];
    Timer timer;
    settings s;
    bool first_run;
};

class FilterControls: public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}

private:
    Ui::UICFilterControls ui;
    settings s;

private slots:
    void doOK();
    void doCancel();
    void update_labels(int);
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("EWMA"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
