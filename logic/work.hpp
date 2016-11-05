/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "main-settings.hpp"
#include "api/plugin-support.hpp"
#include "tracker.h"
#include "shortcuts.h"
#include "export.hpp"
#include "tracklogger.hpp"

#include <QObject>
#include <QFrame>
#include <memory>
#include <vector>
#include <tuple>
#include <functional>

struct OPENTRACK_LOGIC_EXPORT Work
{
    using fn_t = std::function<void(bool)>;
    using key_tuple = std::tuple<key_opts&, fn_t, bool>;
    main_settings s; // tracker needs settings, so settings must come before it
    SelectedLibraries& libs;
    std::shared_ptr<TrackLogger> logger; // must come before tracker, since tracker depends on it
    std::shared_ptr<Tracker> tracker;
    std::shared_ptr<Shortcuts> sc;
    WId handle;
    std::vector<key_tuple> keys;

    Work(Mappings& m, SelectedLibraries& libs, WId handle);
    ~Work();
    void reload_shortcuts();

private:
    static std::shared_ptr<TrackLogger> make_logger(main_settings &s);
    static QString browse_datalogging_file(main_settings &s);
};
