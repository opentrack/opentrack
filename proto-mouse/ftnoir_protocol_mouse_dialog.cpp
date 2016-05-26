#include "ftnoir_protocol_mouse.h"
#include "opentrack/plugin-api.hpp"

MOUSEControls::MOUSEControls()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.Mouse_X, ui.cbxSelectMouse_X);
    tie_setting(s.Mouse_Y, ui.cbxSelectMouse_Y);
}

void MOUSEControls::doOK() {
    s.b->save();
    close();
}

void MOUSEControls::doCancel()
{
    close();
}

