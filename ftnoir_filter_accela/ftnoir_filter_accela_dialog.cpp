#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "opentrack/plugin-api.hpp"

FilterControls::FilterControls() :
    accela_filter(nullptr)
{
    ui.setupUi( this );
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.dampening, ui.dampening_rot_slider);
	tie_setting(s.dampening_translation, ui.dampening_trans_slider);
    tie_setting(s.deadzone, ui.deadzone_slider);
    tie_setting(s.ewma, ui.ewma);
}

void FilterControls::register_filter(IFilter* filter)
{
    accela_filter = static_cast<FTNoIR_Filter*>(filter);
}

void FilterControls::unregister_filter()
{
    accela_filter = nullptr;
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
}

extern "C" OPENTRACK_EXPORT IFilterDialog* GetDialog()
{
    return new FilterControls;
}
