#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "facetracknoir/plugin-support.h"

FilterControls::FilterControls() :
    accela_filter(nullptr)
{
    ui.setupUi( this );
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.rotation_alpha, ui.rotation_alpha);
    tie_setting(s.translation_alpha, ui.translation_alpha);
    tie_setting(s.second_order_alpha, ui.order_2nd);
    tie_setting(s.third_order_alpha, ui.order_3rd);
    tie_setting(s.rot_deadzone, ui.rot_deadzone);
    tie_setting(s.trans_deadzone, ui.trans_deadzone);
    tie_setting(s.expt, ui.expt);
}

void FilterControls::registerFilter(IFilter* filter)
{
    accela_filter = (FTNoIR_Filter*) filter;
}

void FilterControls::unregisterFilter()
{
    accela_filter = NULL;
}

void FilterControls::doOK() {
	save();
	this->close();
}

void FilterControls::doCancel() {
    discard();
    close();
}

void FilterControls::discard()
{
    s.b->reload();
}

void FilterControls::save() {
    s.b->save();
    if (accela_filter)
        accela_filter->receiveSettings();
}

extern "C" OPENTRACK_EXPORT IFilterDialog* GetDialog()
{
    return new FilterControls;
}
