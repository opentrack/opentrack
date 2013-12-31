#include "ftnoir_protocol_libevdev.h"
#include "facetracknoir/global-settings.h"

LibevdevControls::LibevdevControls() : QWidget()
{
	ui.setupUi( this );
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

void LibevdevControls::doOK() {
	save();
	this->close();
}

void LibevdevControls::doCancel() {
    this->close();
}

void LibevdevControls::save() {
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new LibevdevControls;
}
