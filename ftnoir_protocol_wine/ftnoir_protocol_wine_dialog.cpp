#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FTControls::FTControls() : QWidget()
{
	ui.setupUi( this );
	connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
	connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

//
// Destructor for server-dialog

//
// OK clicked on server-dialog
//
void FTControls::doOK() {
	this->close();
}

//
// Cancel clicked on server-dialog
//
void FTControls::doCancel() {
    this->close();
}

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT void* CALLING_CONVENTION GetDialog( )
{
    return (IProtocolDialog*) new FTControls;
}
