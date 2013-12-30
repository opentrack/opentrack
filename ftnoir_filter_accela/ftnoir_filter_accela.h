#pragma once
#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "ui_ftnoir_accela_filtercontrols.h"
#include "facetracknoir/global-settings.h"
#include <QMutex>
#include <QElapsedTimer>

#define ACCELA_SMOOTHING_ROTATION 60.0
#define ACCELA_SMOOTHING_TRANSLATION 40.0
#define ACCELA_SECOND_ORDER_ALPHA 100.0
#define ACCELA_THIRD_ORDER_ALPHA 180.0

#include "facetracknoir/options.hpp"
using namespace options;

class FTNoIR_Filter : public IFilter
{
public:
	FTNoIR_Filter();
    void FilterHeadPoseData(const double* target_camera_position, double *new_camera_position);
    void Initialize() {
        first_run = true;
    }
    void receiveSettings() {
        b->reload();
    }

private:
    QMutex mutex;
	bool first_run;
    pbundle b;
    value<double> rotation_alpha,
                  translation_alpha,
                  second_order_alpha,
                  third_order_alpha,
                  deadzone,
                  expt;
    double last_input[6];
    double last_output[3][6];
    QElapsedTimer timer;
    qint64 frame_delta;
};

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************

// Widget that has controls for FTNoIR protocol filter-settings.
class FilterControls: public QWidget, public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void Initialize(QWidget *);
    void registerFilter(IFilter* filter);
    void unregisterFilter();
private:
    Ui::AccelaUICFilterControls ui;
    void discard();
	void save();
    FTNoIR_Filter* accela_filter;
    pbundle b;
    value<double> rotation_alpha,
                  translation_alpha,
                  second_order_alpha,
                  third_order_alpha,
                  deadzone,
                  expt;
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
