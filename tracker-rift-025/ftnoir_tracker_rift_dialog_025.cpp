#include "ftnoir_tracker_rift_025.h"
#include "api/plugin-api.hpp"

dialog_rift_025::dialog_rift_025()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.constant_drift, ui.constantDrift);
    tie_setting(s.deadzone, ui.deadzone);
    tie_setting(s.persistence, ui.persistence);
    tie_setting(s.useYawSpring, ui.yawSpring);
}

void dialog_rift_025::doOK() {
    s.b->save();
    close();
}

void dialog_rift_025::doCancel() {
    close();
}

