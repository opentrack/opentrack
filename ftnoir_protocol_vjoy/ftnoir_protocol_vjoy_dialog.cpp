#include "ftnoir_protocol_vjoy.h"
#include "facetracknoir/global-settings.h"

VJoyControls::VJoyControls() : QWidget()
{
	ui.setupUi( this );
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

VJoyControls::~VJoyControls() {
}

//
// Initialize tracker-client-dialog
//
void VJoyControls::Initialize(QWidget *parent) {

	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
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

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT IProtocolDialog* CALLING_CONVENTION GetDialog( )
{
    return new VJoyControls;
}
