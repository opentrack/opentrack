/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <QString>
#include "options/options.hpp"
using namespace options;
#include "api/plugin-api.hpp"
#include "spline/axis-opts.hpp"

#include "export.hpp"

struct OTR_LOGIC_EXPORT key_opts
{
    value<QString> keycode, guid;
    value<int> button;

    key_opts(bundle b, const QString& name);
};

struct OTR_LOGIC_EXPORT module_settings
{
    bundle b;
    value<QString> tracker_dll, filter_dll, protocol_dll;
    module_settings();
};

struct OTR_LOGIC_EXPORT main_settings final
{
    bundle b, b_map;
    axis_opts a_x, a_y, a_z;
    axis_opts a_yaw, a_pitch, a_roll;
    std::vector<axis_opts*> all_axis_opts;
    value<bool> tcomp_p, tcomp_disable_tx, tcomp_disable_ty, tcomp_disable_tz;
    value<bool> tcomp_disable_src_yaw, tcomp_disable_src_pitch, tcomp_disable_src_roll;
    value<bool> tray_enabled, tray_start;
    value<bool> center_at_startup;
    //value<int> center_method;
    value<int> neck_z;
    value<bool> neck_enable;
    key_opts key_start_tracking1, key_start_tracking2;
    key_opts key_stop_tracking1, key_stop_tracking2;
    key_opts key_toggle_tracking1, key_toggle_tracking2;
    key_opts key_restart_tracking1, key_restart_tracking2;
    key_opts key_center1, key_center2;
    key_opts key_toggle1, key_toggle2;
    key_opts key_zero1, key_zero2;
    key_opts key_toggle_press1, key_toggle_press2;
    key_opts key_zero_press1, key_zero_press2;
    value<bool> tracklogging_enabled;
    value<QString> tracklogging_filename;

    main_settings();
};
