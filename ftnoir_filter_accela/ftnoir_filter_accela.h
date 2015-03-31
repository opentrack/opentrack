#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "opentrack/plugin-api.hpp"
#include <atomic>
#include <QMutex>
#include <QTimer>

#include "opentrack/options.hpp"
using namespace options;
#include "opentrack/timer.hpp"

struct settings {
    pbundle b;
    value<int> rot_threshold, trans_threshold, ewma;
    settings() :
        b(bundle("Accela")),
        rot_threshold(b, "rotation-threshold", 30),
        trans_threshold(b, "translation-threshold", 50),
        ewma(b, "ewma", 2)
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double* input, double *output);
private:
    settings s;
    bool first_run;
    double last_output[6];
    double smoothed_input[6];
    Timer t;
    static double f(double vec, double thres);
    
    static constexpr double high_thres_c = 4;
    static constexpr double high_thres_out = 500;
    
    static constexpr double low_thres_mult = 100;
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
private slots:
    void doOK();
    void doCancel();
    void update_ewma_display(int value);
    void update_rot_display(int value);
    void update_trans_display(int value);
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Accela"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
