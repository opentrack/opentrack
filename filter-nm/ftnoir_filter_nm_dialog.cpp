/* Copyright (c) 2023 Tom Brazier <tom_github@firstsolo.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_nm.h"

using namespace options;

dialog_nm::dialog_nm()
{
    ui.setupUi(this);

    tie_setting(s.pos_responsiveness, ui.pos_responsiveness_slider);
    tie_setting(s.pos_responsiveness, ui.pos_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.rot_responsiveness, ui.rot_responsiveness_slider);
    tie_setting(s.rot_responsiveness, ui.rot_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.pos_drift_speed, ui.pos_drift_speed_slider);
    tie_setting(s.pos_drift_speed, ui.pos_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.rot_drift_speed, ui.rot_drift_speed_slider);
    tie_setting(s.rot_drift_speed, ui.rot_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });
}

void dialog_nm::doOK()
{
    save();
    close();
}

void dialog_nm::doCancel()
{
    close();
}

void dialog_nm::save()
{
    s.b->save();
}

void dialog_nm::reload()
{
    s.b->reload();
}

void dialog_nm::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
}
