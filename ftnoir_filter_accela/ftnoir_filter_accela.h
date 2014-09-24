#pragma once
#include "ui_ftnoir_accela_filtercontrols.h"
#include "facetracknoir/plugin-api.hpp"
#include <QMutex>

#define ACCELA_SMOOTHING_ROTATION 60.0
#define ACCELA_SMOOTHING_TRANSLATION 40.0
#define ACCELA_SECOND_ORDER_ALPHA 100.0
#define ACCELA_THIRD_ORDER_ALPHA 180.0

#include <facetracknoir/options.h>
using namespace options;

struct settings {
    pbundle b;
    value<double> rotation_alpha,
                  translation_alpha,
                  second_order_alpha,
                  third_order_alpha,
                  rot_deadzone,
                  trans_deadzone,
                  expt;
    settings() :
        b(bundle("Accela")),
        rotation_alpha(b, "rotation-alpha", ACCELA_SMOOTHING_ROTATION),
        translation_alpha(b, "translation-alpha", ACCELA_SMOOTHING_TRANSLATION),
        second_order_alpha(b, "second-order-alpha", ACCELA_SECOND_ORDER_ALPHA),
        third_order_alpha(b, "third-order-alpha", ACCELA_THIRD_ORDER_ALPHA),
        rot_deadzone(b, "rotation-deadband", 0),
        trans_deadzone(b, "translation-deadband", 0),
        expt(b, "exponent", 2)
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
	FTNoIR_Filter();
    void FilterHeadPoseData(const double* target_camera_position, double *new_camera_position);
    void reset() {
        first_run = true;
    }
    void receiveSettings() {
        s.b->reload();
    }

private:
    settings s;
	bool first_run;
    double last_output[3][6];
};

class FilterControls: public QWidget, public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void registerFilter(IFilter* filter);
    void unregisterFilter();
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
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Accela Filter Mk4"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Accela Mk4"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Accela filter Mk4"); }

    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png");	}
};
