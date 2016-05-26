#include "ftnoir_protocol_vjoy.h"
#include "opentrack/plugin-api.hpp"

VJoyControls::VJoyControls()
{
	ui.setupUi( this );
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

void VJoyControls::doOK() {
	save();
	close();
}

void VJoyControls::doCancel() {
    close();
}

void VJoyControls::save() {
}

