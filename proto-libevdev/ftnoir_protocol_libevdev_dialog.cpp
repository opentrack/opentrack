#include "ftnoir_protocol_libevdev.h"
#include "api/plugin-api.hpp"

LibevdevControls::LibevdevControls()
{
	ui.setupUi( this );
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

void LibevdevControls::doOK() {
	save();
	close();
}

void LibevdevControls::doCancel() {
    close();
}

void LibevdevControls::save() {
}
