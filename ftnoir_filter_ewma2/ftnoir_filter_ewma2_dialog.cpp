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
    
    timer.setInterval(500);
    
    connect(&timer, SIGNAL(timeout()), this, SLOT(show_state()));
}

void FilterControls::show_state()
{
    State s;
    
    if (pFilter)
        s = pFilter->get_state();
    
    QDoubleSpinBox* dbs[] = {
        ui.d0, ui.d1, ui.d2, ui.d3, ui.d4, ui.d5,
        ui.n0, ui.n1, ui.n2, ui.n3, ui.n4, ui.n5,
    };
    
    for (int i = 0; i < 6; i++)
        dbs[i]->setValue(s.delta[i]);
    
    for (int i = 0; i < 6; i++)
        dbs[i+6]->setValue(s.noise[i]);
}

void FilterControls::registerFilter(IFilter* flt)
{
    pFilter = (FTNoIR_Filter*) flt;
    timer.start();
}

void FilterControls::unregisterFilter()
{
    pFilter = NULL;
    timer.stop();
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
