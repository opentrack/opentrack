/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include "options/options.hpp"
#include "api/plugin-api.hpp"

using namespace options;

#include "export.hpp"

struct axis_opts final
{
    // note, these two bundles can be the same value with no issues
    bundle b_settings_window, b_mapping_window;
    value<double> zero;
    value<int> src;
    value<bool> invert, altp;
    axis_opts(bundle b_settings_window, bundle b_mapping_window, QString pfx, int idx) :
        b_settings_window(b_settings_window),
        b_mapping_window(b_mapping_window),
        zero(b_settings_window, n(pfx, "zero-pos"), 0),
        src(b_settings_window, n(pfx, "source-index"), idx),
        invert(b_settings_window, n(pfx, "invert-sign"), false),
        altp(b_mapping_window, n(pfx, "alt-axis-sign"), false)
    {}
private:
    static inline QString n(QString pfx, QString name)
    {
        return QString("%1-%2").arg(pfx, name);
    }
};

struct key_opts
{
    value<QString> keycode, guid;
    value<int> button;

    key_opts(bundle b, const QString& name) :
        keycode(b, QString("keycode-%1").arg(name), ""),
        guid(b, QString("guid-%1").arg(name), ""),
        button(b, QString("button-%1").arg(name), -1)
    {}
};

struct module_settings
{
    bundle b;
    value<QString> tracker_dll, filter_dll, protocol_dll;
    module_settings() :
        b(make_bundle("modules")),
        tracker_dll(b, "tracker-dll", "PointTracker 1.1"),
        filter_dll(b, "filter-dll", "Accela"),
        protocol_dll(b, "protocol-dll", "freetrack 2.0 Enhanced")
    {
    }
};

struct main_settings
{
    bundle b, b_map;
    axis_opts a_x, a_y, a_z, a_yaw, a_pitch, a_roll;
    value<bool> tcomp_p, tcomp_disable_tx, tcomp_disable_ty, tcomp_disable_tz;
    value<bool> tcomp_disable_src_yaw, tcomp_disable_src_pitch, tcomp_disable_src_roll;
    value<bool> tray_enabled, tray_start;
    value<int> camera_yaw, camera_pitch, camera_roll;
    value<bool> use_camera_offset_from_centering;
    value<bool> center_at_startup;
    value<int> center_method;
    key_opts key_start_tracking, key_stop_tracking, key_toggle_tracking, key_restart_tracking;
    key_opts key_center, key_toggle, key_zero;
    key_opts key_toggle_press, key_zero_press;
    key_opts key_disable_tcomp_press;
    value<bool> tracklogging_enabled;
    value<QString> tracklogging_filename;
    main_settings() :
        b(make_bundle("opentrack-ui")),
        b_map(make_bundle("opentrack-mappings")),
        a_x(b, b_map, "x", TX),
        a_y(b, b_map, "y", TY),
        a_z(b, b_map, "z", TZ),
        a_yaw(b, b_map, "yaw", Yaw),
        a_pitch(b, b_map, "pitch", Pitch),
        a_roll(b, b_map, "roll", Roll),
        tcomp_p(b, "compensate-translation", true),
        tcomp_disable_tx(b, "compensate-translation-disable-x-axis", false),
        tcomp_disable_ty(b, "compensate-translation-disable-y-axis", false),
        tcomp_disable_tz(b, "compensate-translation-disable-z-axis", false),
        tcomp_disable_src_yaw(b, "compensate-translation-disable-source-yaw", false),
        tcomp_disable_src_pitch(b, "compensate-translation-disable-source-pitch", false),
        tcomp_disable_src_roll(b, "compensate-translation-disable-source-roll", false),
        tray_enabled(b, "use-system-tray", false),
        tray_start(b, "start-in-tray", false),
        camera_yaw(b, "camera-yaw", 0),
        camera_pitch(b, "camera-pitch", 0),
        camera_roll(b, "camera-roll", 0),
        use_camera_offset_from_centering(b, "use-camera-offset-from-centering", false),
        center_at_startup(b, "center-at-startup", true),
        center_method(b, "centering-method", true),
        key_start_tracking(b, "start-tracking"),
        key_stop_tracking(b, "stop-tracking"),
        key_toggle_tracking(b, "toggle-tracking"),
        key_restart_tracking(b, "restart-tracking"),
        key_center(b, "center"),
        key_toggle(b, "toggle"),
        key_zero(b, "zero"),
        key_toggle_press(b, "toggle-press"),
        key_zero_press(b, "zero-press"),
        key_disable_tcomp_press(b, "disable-translation-compensation-while-held"),
        tracklogging_enabled(b, "tracklogging-enabled", false),
        tracklogging_filename(b, "tracklogging-filename", QString())
    {
    }
};
