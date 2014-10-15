#pragma once

#include "facetracknoir/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include "facetracknoir/options.h"
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
    void reset() {}
    void FilterHeadPoseData(const double *target_camera_position,
                            double *new_camera_position);
    void receiveSettings();
private:
    bool first_run;
    double delta_smoothing;
    double noise_smoothing;
    double delta[6];
    double noise[6];
    double current_camera_position[6];
    settings s;
};

class FilterControls: public QWidget, public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void registerFilter(IFilter* flt);
    void unregisterFilter();

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
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("EWMA Filter Mk3"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("EWMA"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Exponentially Weighted Moving Average filter with dynamic smoothing parameter"); }
    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png"); }
};
