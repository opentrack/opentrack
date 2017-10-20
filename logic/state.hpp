/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
using namespace options;
#include "api/plugin-support.hpp"
#include "main-settings.hpp"
#include "mappings.hpp"
#include "extensions.hpp"
#include "work.hpp"
#include <vector>
#include <QString>

struct State
{
    State(const QString& library_path) :
        modules(library_path),
        ev(modules.extensions()),
        pose(s.all_axis_opts)
    {}
    Modules modules;
    event_handler ev;
    main_settings s;
    Mappings pose;
    std::shared_ptr<Work> work;
};
