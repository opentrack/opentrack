#include "ftnoir_tracker_freepie-udp.h"
#include "api/plugin-api.hpp"

dialog_freepie::dialog_freepie()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.port);
    tie_setting(s.idx_x, ui.input_x);
    tie_setting(s.idx_y, ui.input_y);
    tie_setting(s.idx_z, ui.input_z);

    tie_setting(s.add_yaw, ui.add_yaw);
    tie_setting(s.add_pitch, ui.add_pitch);
    tie_setting(s.add_roll, ui.add_roll);
}

void dialog_freepie::doOK() {
    s.b->save();
    close();
}

void dialog_freepie::doCancel()
{
    close();
}

