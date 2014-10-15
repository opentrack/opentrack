#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

FTControls::FTControls() : QWidget()
{
    ui.setupUi( this );
    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
}

void FTControls::doOK() {
    this->close();
}

void FTControls::doCancel() {
    this->close();
}

extern "C" OPENTRACK_EXPORT void* GetDialog( )
{
    return (IProtocolDialog*) new FTControls;
}
