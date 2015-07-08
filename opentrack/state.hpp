/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include <vector>
#include "opentrack/options.hpp"
using namespace options;
#include "opentrack/plugin-support.hpp"
#include "opentrack/main-settings.hpp"
#include "opentrack/mappings.hpp"
#include "opentrack/selected-libraries.hpp"
#include "opentrack/work.hpp"

struct State {
    State() :
        pose(std::vector<axis_opts*>{&s.a_x, &s.a_y, &s.a_z, &s.a_yaw, &s.a_pitch, &s.a_roll})
    {}
    Modules modules;
    SelectedLibraries libs;
    main_settings s;
    Mappings pose;
    mem<Work> work;
};
