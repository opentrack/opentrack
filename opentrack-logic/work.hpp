/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "main-settings.hpp"
#include "opentrack/plugin-support.hpp"
#include "tracker.h"
#include "shortcuts.h"

#include <QObject>
#include <QFrame>
#include <memory>
#include <vector>
#include <functional>
#include <tuple>

struct Work
{
    main_settings s;
    SelectedLibraries& libs;
    mem<Tracker> tracker;
    mem<Shortcuts> sc;
    WId handle;
    using fn = std::function<void(bool)>;
    using tt = std::tuple<key_opts&, fn, bool>;
    std::vector<tt> keys;

    Work(Mappings& m, SelectedLibraries& libs, WId handle) :
        libs(libs),
        tracker(std::make_shared<Tracker>(m, libs)),
        sc(std::make_shared<Shortcuts>()),
        handle(handle),
        keys {
            tt(s.key_center, [&](bool) -> void { tracker->center(); }, true),
            tt(s.key_toggle, [&](bool) -> void { tracker->toggle_enabled(); }, true),
            tt(s.key_zero, [&](bool) -> void { tracker->zero(); }, true),
            tt(s.key_toggle_press, [&](bool x) -> void { tracker->set_toggle(!x); }, false),
            tt(s.key_zero_press, [&](bool x) -> void { tracker->set_zero(x); }, false),
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
