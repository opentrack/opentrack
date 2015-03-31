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
    value<int> dampening, dampening_translation, deadzone, ewma;
    settings() :
        b(bundle("Accela")),
        dampening(b, "dampening", 30), // rotation
		dampening_translation(b, "dampening_translation", 30),
        deadzone(b, "deadzone", 50),
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
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Accela"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
