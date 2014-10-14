#include "ftnoir_filter_ewma2.h"
#include <cmath>
#include <QDebug>
#include "facetracknoir/plugin-support.h"
#include "ui_ftnoir_ewma_filtercontrols.h"

FilterControls::FilterControls() :
    QWidget(), pFilter(NULL)
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.kMaxSmoothing, ui.maxSmooth);
    tie_setting(s.kMinSmoothing, ui.minSmooth);
    tie_setting(s.kSmoothingScaleCurve, ui.powCurve);
}

void FilterControls::registerFilter(IFilter* flt)
{
    pFilter = (FTNoIR_Filter*) flt;
}

void FilterControls::unregisterFilter()
{
    pFilter = NULL;
}

void FilterControls::doOK() {
	save();
	this->close();
}

void FilterControls::doCancel() {
    s.b->reload();
    this->close();
}

void FilterControls::save() {
    s.b->save();
    if (pFilter)
        pFilter->receiveSettings();
}

extern "C" OPENTRACK_EXPORT IFilterDialog* GetDialog( )
{
    return new FilterControls;
}
