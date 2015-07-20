/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include "opentrack/options.hpp"
#include "opentrack/plugin-api.hpp"

using namespace options;

struct axis_opts {
    value<bool> invert, altp;
    value<int> src;
    axis_opts(pbundle b, QString pfx, int idx) :
        invert(b, n(pfx, "invert-sign"), false),
        altp(b, n(pfx, "alt-axis-sign"), false),
        src(b, n(pfx, "source-index"), idx)
    {}
private:
    static inline QString n(QString pfx, QString name) {
        return QString("%1-%2").arg(pfx, name);
    }
};

struct main_settings : opts {
    value<QString> protocol_dll;
    axis_opts a_x, a_y, a_z, a_yaw, a_pitch, a_roll;
    value<bool> tcomp_p, tcomp_tz;
    value<bool> tray_enabled;
    value<int> camera_yaw, camera_pitch, camera_roll;
    value<bool> center_at_startup, wizard_done;
    main_settings() :
        opts("opentrack-ui"),
        protocol_dll(b, "protocol-dll", ""),
        a_x(b, "x", TX),
        a_y(b, "y", TY),
        a_z(b, "z", TZ),
        a_yaw(b, "yaw", Yaw),
        a_pitch(b, "pitch", Pitch),
        a_roll(b, "roll", Roll),
        tcomp_p(b, "compensate-translation", true),
        tcomp_tz(b, "compensate-translation-disable-z-axis", false),
        tray_enabled(b, "use-system-tray", false),
        camera_yaw(b, "camera-yaw", 0),
        camera_pitch(b, "camera-pitch", 0),
        camera_roll(b, "camera-roll", 0),
        center_at_startup(b, "center-at-startup", true),
        wizard_done(b, "wizard-done", false)
    {}
};
