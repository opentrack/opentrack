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
#include "spline/axis-opts.hpp"
#include "input/key-opts.hpp"

#include "export.hpp"

enum reltrans_state : int
{
    reltrans_disabled   = 0,
    reltrans_enabled    = 1,
    reltrans_non_center = 2,
};

enum centering_state : int
{
    center_disabled         = 0,
    center_point            = 1,
    center_vr360            = 2,
    center_roll_compensated = 3,
};

namespace main_settings_impl {

using namespace options;

struct OTR_LOGIC_EXPORT module_settings
{
    bundle b { make_bundle("modules") };
    value<QString> tracker_dll { b, "tracker-dll", "pt" };
    value<QString> filter_dll { b, "filter-dll", "accela" };
    value<QString> protocol_dll { b, "protocol-dll", "freetrack" };
    module_settings();
};

struct OTR_LOGIC_EXPORT main_settings final
{
    bundle b { make_bundle("opentrack-ui") };
    bundle b_map { make_bundle("opentrack-mappings") };
    axis_opts a_x{ "x", TX }, a_y { "y", TY }, a_z { "z", TZ };
    axis_opts a_yaw{ "yaw", Yaw }, a_pitch { "pitch", Pitch }, a_roll { "roll", Roll };
    axis_opts* all_axis_opts[6] { &a_x, &a_y, &a_z, &a_yaw, &a_pitch, &a_roll };
    value<reltrans_state> reltrans_mode { b, "relative-translation-mode", reltrans_disabled };

    value<bool> reltrans_disable_tx { b, "compensate-translation-disable-x-axis", false };
    value<bool> reltrans_disable_ty { b, "compensate-translation-disable-y-axis", false };
    value<bool> reltrans_disable_tz { b, "compensate-translation-disable-z-axis", false };

    value<bool> reltrans_disable_src_yaw { b, "compensate-translation-disable-source-yaw", false };
    value<bool> reltrans_disable_src_pitch { b, "compensate-translation-disable-source-pitch", false };
    value<bool> reltrans_disable_src_roll { b, "compensate-translation-disable-source-roll", false };

    value<bool> enable_camera_offset { b, "enable-camera-offset", false };

    value<int> camera_offset_yaw   { b, "camera-offset-yaw",   0 };
    value<int> camera_offset_pitch { b, "camera-offset-pitch", 0 };
    value<int> camera_offset_roll  { b, "camera-offset-roll",  0 };

    value<bool> tray_enabled { b, "use-system-tray", false };
    value<bool> tray_start { b, "start-in-tray", false };

    value<bool> center_at_startup { b, "center-at-startup", true };
    value<centering_state> centering_mode { b, "centering-mode", center_roll_compensated };
    value<int> neck_z { b, "neck-depth", 0 };
    value<bool> neck_enable { b, "neck-enable", false };

    key_opts key_start_tracking1 { b, "start-tracking" };
    key_opts key_start_tracking2 { b, "start-tracking-alt" };

    key_opts key_stop_tracking1 { b, "stop-tracking" };
    key_opts key_stop_tracking2 { b, "stop-tracking-alt" };

    key_opts key_toggle_tracking1 { b, "toggle-tracking" };
    key_opts key_toggle_tracking2 { b, "toggle-tracking-alt" };

    key_opts key_restart_tracking1 { b, "restart-tracking" };
    key_opts key_restart_tracking2 { b, "restart-tracking-alt" };

    key_opts key_center1 { b, "center" };
    key_opts key_center2 { b, "center-alt" };

    key_opts key_toggle1 { b, "toggle" };
    key_opts key_toggle2 { b, "toggle-alt" };

    key_opts key_zero1 { b, "zero" };
    key_opts key_zero2 { b, "zero-alt" };

    key_opts key_toggle_press1 { b, "toggle-press" };
    key_opts key_toggle_press2 { b, "toggle-press-alt" };

    key_opts key_zero_press1 { b, "zero-press" };
    key_opts key_zero_press2 { b, "zero-press-alt" };

    value<bool> tracklogging_enabled { b, "tracklogging-enabled", false };
    value<QString> tracklogging_filename { b, "tracklogging-filename", {} };

    main_settings();
};

} // ns main_settings_impl

using module_settings = main_settings_impl::module_settings;
using main_settings = main_settings_impl::main_settings;
