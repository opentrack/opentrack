#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "opentrack/plugin-api.hpp"
#include "qfunctionconfigurator/functionconfig.h"
#include <atomic>
#include <QMutex>
#include <QTimer>

#include "opentrack/options.hpp"
using namespace options;
#include "opentrack/timer.hpp"

struct settings_accela : opts {
    value<int> rot_threshold, trans_threshold, ewma, rot_deadzone, trans_deadzone;
    static constexpr double mult_rot = 10. / 100.;
    static constexpr double mult_trans = 5. / 100.;
    static constexpr double mult_rot_dz = 2. / 100.;
    static constexpr double mult_trans_dz = 1. / 100.;
    static constexpr double mult_ewma = 2.;
    settings_accela() :
        opts("Accela"),
        rot_threshold(b, "rotation-threshold", 30),
        trans_threshold(b, "translation-threshold", 50),
        ewma(b, "ewma", 2),
        rot_deadzone(b, "rotation-deadzone", 0),
        trans_deadzone(b, "translation-deadzone", 0)
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void filter(const double* input, double *output);
    Map rot, trans;
private:
    settings_accela s;
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
    settings_accela s;
private slots:
    void doOK();
    void doCancel();
    void update_ewma_display(int value);
    void update_rot_display(int value);
    void update_trans_display(int value);
    void update_rot_dz_display(int value);
    void update_trans_dz_display(int value);
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Accela"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
