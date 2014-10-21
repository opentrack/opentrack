#pragma once

#include "opentrack/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include "opentrack/options.hpp"
using namespace options;

struct settings {
    pbundle b;
    // these are sadly sliders for now due to int/double mismatch -sh
    value<int> kMinSmoothing, kMaxSmoothing, kSmoothingScaleCurve;
    settings() :
        b(bundle("ewma-filter")),
        kMinSmoothing(b, "min-smoothing", 15),
        kMaxSmoothing(b, "max-smoothing", 50),
        kSmoothingScaleCurve(b, "smoothing-scale-curve", 10)
    {}
};


class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void reset() { first_run=true; }
    void filter(const double *target_camera_position,
                            double *new_camera_position);
    void receiveSettings();
private:
    bool first_run;
    double delta_alpha;
    double noise_alpha;
    double delta[6];
    double noise[6];
    double output[6];
    settings s;
};

class FilterControls: public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void register_filter(IFilter* flt);
    void unregister_filter();

private:
    Ui::UICFilterControls ui;
    void save();
    settings s;
    FTNoIR_Filter* pFilter;

private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("EWMA"); }
   QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
