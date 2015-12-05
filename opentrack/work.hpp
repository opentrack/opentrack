/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "opentrack/main-settings.hpp"
#include "opentrack/plugin-support.hpp"
#include "opentrack/tracker.h"
#include "opentrack/shortcuts.h"

#include <QObject>
#include <QFrame>
#include <memory>
#include <vector>
#include <functional>
#include <tuple>

struct Work
{
    main_settings& s;
    SelectedLibraries& libs;
    mem<Tracker> tracker;
    mem<Shortcuts> sc;
    WId handle;
    using fn = std::function<void(void)>;
    using tt = std::tuple<key_opts&, fn>;
    std::vector<std::tuple<key_opts&, fn>> keys;
    
    Work(main_settings& s, Mappings& m, SelectedLibraries& libs, WId handle) :
        s(s), libs(libs),
        tracker(std::make_shared<Tracker>(s, m, libs)),
        sc(std::make_shared<Shortcuts>()),
        handle(handle),
        keys {
            tt(s.key_center, [&]() -> void { tracker->center(); }),
            tt(s.key_toggle, [&]() -> void { tracker->toggle_enabled(); }),
            tt(s.key_zero, [&]() -> void { tracker->zero(); }),
        }
    {
        reload_shortcuts();
        tracker->start();   
    }

    void reload_shortcuts()
    {
        sc->reload(keys);
    }

    ~Work()
    {
        sc = nullptr;
        // order matters, otherwise use-after-free -sh
        tracker = nullptr;
        libs = SelectedLibraries();
    }
};
