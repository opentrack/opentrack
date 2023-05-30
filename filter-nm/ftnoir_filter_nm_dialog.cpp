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

    tie_setting(s.responsiveness[0], ui.x_responsiveness_slider);
    tie_setting(s.responsiveness[0], ui.x_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.responsiveness[1], ui.y_responsiveness_slider);
    tie_setting(s.responsiveness[1], ui.y_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.responsiveness[2], ui.z_responsiveness_slider);
    tie_setting(s.responsiveness[2], ui.z_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.responsiveness[3], ui.yaw_responsiveness_slider);
    tie_setting(s.responsiveness[3], ui.yaw_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.responsiveness[4], ui.pitch_responsiveness_slider);
    tie_setting(s.responsiveness[4], ui.pitch_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.responsiveness[5], ui.roll_responsiveness_slider);
    tie_setting(s.responsiveness[5], ui.roll_responsiveness, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.drift_speeds[0], ui.x_drift_speed_slider);
    tie_setting(s.drift_speeds[0], ui.x_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.drift_speeds[1], ui.y_drift_speed_slider);
    tie_setting(s.drift_speeds[1], ui.y_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.drift_speeds[2], ui.z_drift_speed_slider);
    tie_setting(s.drift_speeds[2], ui.z_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.drift_speeds[3], ui.yaw_drift_speed_slider);
    tie_setting(s.drift_speeds[3], ui.yaw_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.drift_speeds[4], ui.pitch_drift_speed_slider);
    tie_setting(s.drift_speeds[4], ui.pitch_drift_speed, [](double x)
        { return QStringLiteral("%1").arg(x, 0, 'f', 2); });

    tie_setting(s.drift_speeds[5], ui.roll_drift_speed_slider);
    tie_setting(s.drift_speeds[5], ui.roll_drift_speed, [](double x)
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
