#include "ftnoir_tracker_udp.h"
#include "api/plugin-api.hpp"

dialog_udp::dialog_udp()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.spinPortNumber);
    tie_setting(s.add_yaw, ui.add_yaw);
    tie_setting(s.add_pitch, ui.add_pitch);
    tie_setting(s.add_roll, ui.add_roll);
}

void dialog_udp::doOK() {
    s.b->save();
    close();
}

void dialog_udp::doCancel() {
    close();
}

