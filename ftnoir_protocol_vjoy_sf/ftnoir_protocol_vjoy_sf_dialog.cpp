#include "ftnoir_protocol_vjoy_sf.h"
#include "facetracknoir/plugin-support.h"

VJoySFControls::VJoySFControls() : QWidget()
{
    ui.setupUi( this );
    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

    tie_setting(s.intvJoyID, ui.sbvJoyID);
}

void VJoySFControls::doOK() {
    save();
    this->close();
}

void VJoySFControls::doCancel() {
    revert();
    this->close();
}

void VJoySFControls::save() {
    s.b->save();
}

void VJoySFControls::revert() {
    s.b->revert();
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new VJoySFControls;
}
