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

    tie_setting(s.rot_deadzone, ui.rot_deadzone);
    tie_setting(s.trans_deadzone, ui.trans_deadzone);

    tie_setting(s.rot_plus , ui.rot_plus);
    tie_setting(s.rot_minus , ui.rot_minus);
    tie_setting(s.trans_smoothing, ui.trans_smoothing);

    connect(&t, SIGNAL(timeout()), this, SLOT(timer_fired()));

    t.setInterval(250);
}

void FilterControls::timer_fired()
{
    if (accela_filter)
    {
        state_display st = accela_filter->state;
        ui.debug_y->setValue(st.y);
        ui.debug_p->setValue(st.p);
        ui.debug_r->setValue(st.r);
    }
}

void FilterControls::register_filter(IFilter* filter)
{
    accela_filter = static_cast<FTNoIR_Filter*>(filter);
    t.start();
}

void FilterControls::unregister_filter()
{
    t.stop();
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
