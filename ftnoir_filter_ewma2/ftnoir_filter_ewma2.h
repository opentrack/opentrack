#pragma once

#include "facetracknoir/plugin-api.hpp"
#include "ui_ftnoir_ewma_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    // these are sadly sliders for now due to int/double mismatch -sh
    value<int> kMinSmoothing, kMaxSmoothing;
    settings() :
        b(bundle("ewma-filter")),
        kMinSmoothing(b, "min-smoothing", 15),
        kMaxSmoothing(b, "max-smoothing", 50)
    {}
};

struct State {
    State() :
        delta{-1, -1, -1, -1, -1, -1},
        noise{-1, -1, -1, -1, -1, -1},
        last_output{-1, -1, -1, -1, -1, -1}
    {}
    double delta[6];
    double noise[6];
    double last_output[6];
};

class FTNoIR_Filter : public IFilter, private State
{
public:
    FTNoIR_Filter();
    void FilterHeadPoseData(const double *input, double *output);
    void receiveSettings();
    State get_state() { QMutexLocker l(&state_mutex); return State(*static_cast<State*>(this)); }
private:
    bool first_run;
    double delta_alpha;
    double noise_alpha;
    settings s;
    QMutex state_mutex;
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
    QTimer timer;
public slots:
    void show_state();
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
