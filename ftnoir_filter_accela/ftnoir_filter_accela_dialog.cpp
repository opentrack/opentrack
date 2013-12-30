#include "ftnoir_filter_accela/ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "facetracknoir/global-settings.h"

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
    tie_setting(s.deadzone, ui.deadzone);
    tie_setting(s.expt, ui.expt);
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
    if (!s.b->modifiedp())
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
    s.b->revert();
}

void FilterControls::save() {
    s.b->save();
    if (accela_filter)
        accela_filter->receiveSettings();
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilterDialog* CALLING_CONVENTION GetDialog()
{
    return new FilterControls;
}
