/* Copyright (c) 2025
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "ftnoir_tracker_smoothtrack.h"
#include "api/plugin-api.hpp"

dialog_smoothtrack::dialog_smoothtrack()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.spinPortNumber);
    tie_setting(s.add_yaw, ui.add_yaw);
    tie_setting(s.add_pitch, ui.add_pitch);
    tie_setting(s.add_roll, ui.add_roll);
}

void dialog_smoothtrack::doOK() {
    s.b->save();
    close();
}

void dialog_smoothtrack::doCancel() {
    close();
}
