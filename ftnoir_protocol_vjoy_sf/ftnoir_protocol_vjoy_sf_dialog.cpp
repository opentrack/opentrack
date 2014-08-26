#include "ftnoir_protocol_vjoy_sf.h"
#include "facetracknoir/plugin-support.h"

VJoySFControls::VJoySFControls() : QWidget()
{
    ui.setupUi( this );
    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

void VJoySFControls::doOK() {
    save();
    this->close();
}

void VJoySFControls::doCancel() {
    this->close();
}

void VJoySFControls::save() {
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new VJoySFControls;
}
