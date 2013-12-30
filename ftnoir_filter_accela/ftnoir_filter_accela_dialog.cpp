#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "facetracknoir/global-settings.h"

FilterControls::FilterControls() :
    accela_filter(NULL),
    b(bundle("Accela")),
    rotation_alpha(b, "rotation-alpha", ACCELA_SMOOTHING_ROTATION),
    translation_alpha(b, "translation-alpha", ACCELA_SMOOTHING_TRANSLATION),
    second_order_alpha(b, "second-order-alpha", ACCELA_SECOND_ORDER_ALPHA),
    third_order_alpha(b, "third-order-alpha", ACCELA_THIRD_ORDER_ALPHA),
    deadzone(b, "deadzone", 0),
    expt(b, "exponent", 2)

{
    ui.setupUi( this );
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(rotation_alpha, ui.rotation_alpha);
    tie_setting(translation_alpha, ui.translation_alpha);
    tie_setting(second_order_alpha, ui.order_2nd);
    tie_setting(third_order_alpha, ui.order_3rd);
    tie_setting(deadzone, ui.deadzone);
    tie_setting(expt, ui.expt);
}

void FilterControls::Initialize(QWidget *) {
	show();
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
    if (!b->modifiedp())
    {
        close();
        return;
    }
    int ret =
            QMessageBox::question( this,
                                   "Settings have changed",
                                   "Do you want to save the settings?",
                                   QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);

    switch (ret) {
    case QMessageBox::Save:
        save();
        this->close();
        break;
    case QMessageBox::Discard:
        this->discard();
        this->close();
        break;
    case QMessageBox::Cancel:
    default:
        break;
    }
}


void FilterControls::discard()
{
    b->revert();
}

void FilterControls::save() {
    b->save();
    if (accela_filter)
        accela_filter->receiveSettings();
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilterDialog* CALLING_CONVENTION GetDialog()
{
    return new FilterControls;
}
