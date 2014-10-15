#include "ftnoir_protocol_vjoy.h"
#include "facetracknoir/plugin-support.h"

VJoyControls::VJoyControls() : QWidget()
{
	ui.setupUi( this );
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

void VJoyControls::doOK() {
	save();
	this->close();
}

void VJoyControls::doCancel() {
    this->close();
}

void VJoyControls::save() {
}

extern "C" OPENTRACK_EXPORT IProtocolDialog* GetDialog( )
{
    return new VJoyControls;
}
