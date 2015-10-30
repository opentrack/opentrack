#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include "opentrack/plugin-api.hpp"

FTControls::FTControls()
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

