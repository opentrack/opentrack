#include "livelink.h"
#include "api/plugin-api.hpp"

dialog_livelink::dialog_livelink()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.spinPortNumber);
    tie_setting(s.add_yaw, ui.add_yaw);
    tie_setting(s.add_pitch, ui.add_pitch);
    tie_setting(s.add_roll, ui.add_roll);
}

void dialog_livelink::doOK() {
    s.b->save();
    close();
}

void dialog_livelink::doCancel() {
    close();
}
