/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include "opentrack-compat/options.hpp"
#include "opentrack/plugin-api.hpp"

using namespace options;

struct axis_opts {
    pbundle b;
    value<double> zero;
    value<bool> invert, altp;
    value<int> src;
    axis_opts(pbundle b, QString pfx, int idx) :
        b(b),
        zero(b, n(pfx, "zero-pos"), 0),
        invert(b, n(pfx, "invert-sign"), false),
        altp(b, n(pfx, "alt-axis-sign"), false),
        src(b, n(pfx, "source-index"), idx)
    {}
private:
    static inline QString n(QString pfx, QString name) {
        return QString("%1-%2").arg(pfx, name);
    }
};

struct key_opts {
    value<QString> keycode, guid;
    value<int> button;

    key_opts(pbundle b, const QString& name) :
        keycode(b, QString("keycode-%1").arg(name), ""),
        guid(b, QString("guid-%1").arg(name), ""),
        button(b, QString("button-%1").arg(name), -1)
    {}
};

struct main_settings : opts {    
    value<QString> tracker_dll, filter_dll, protocol_dll;
    axis_opts a_x, a_y, a_z, a_yaw, a_pitch, a_roll;
    value<bool> tcomp_p, tcomp_tz;
    value<bool> tray_enabled;
    value<int> camera_yaw, camera_pitch, camera_roll;
    value<bool> center_at_startup;
    value<int> center_method;
    key_opts key_start_tracking, key_stop_tracking, key_toggle_tracking;
    key_opts key_center, key_toggle, key_zero;
    main_settings() :
        opts("opentrack-ui"),
        tracker_dll(b, "tracker-dll", ""),
        filter_dll(b, "filter-dll", "Accela"),
        protocol_dll(b, "protocol-dll", "freetrack 2.0 Enhanced"),
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
        center_method(b, "centering-method", true),
        key_start_tracking(b, "start-tracking"),
        key_stop_tracking(b, "stop-tracking"),
        key_toggle_tracking(b, "toggle-tracking"),
        key_center(b, "center"),
        key_toggle(b, "toggle"),
        key_zero(b, "zero")
    {
    }
};
