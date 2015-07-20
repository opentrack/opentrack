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

struct Work
{
    main_settings& s;
    SelectedLibraries& libs;
    mem<Tracker> tracker;
    mem<Shortcuts> sc;
    WId handle;

    Work(main_settings& s, Mappings& m, SelectedLibraries& libs, QObject* recv, WId handle) :
        s(s), libs(libs),
        tracker(std::make_shared<Tracker>(s, m, libs)),
        sc(std::make_shared<Shortcuts>(handle)),
        handle(handle)
    {
#ifndef _WIN32
        QObject::connect(sc->keyCenter.get(), SIGNAL(activated()), recv, SLOT(shortcutRecentered()));
        QObject::connect(sc->keyToggle.get(), SIGNAL(activated()), recv, SLOT(shortcutToggled()));
        QObject::connect(sc->keyZero.get(), SIGNAL(activated()), recv, SLOT(shortcutZeroed()));
#else
        QObject::connect(sc.get(), SIGNAL(center()), recv, SLOT(shortcutRecentered()));
        QObject::connect(sc.get(), SIGNAL(toggle()), recv, SLOT(shortcutToggled()));
        QObject::connect(sc.get(), SIGNAL(zero()), recv, SLOT(shortcutZeroed()));
#endif
        tracker->start();
    }

    void reload_shortcuts()
    {
        sc->reload();
    }

    ~Work()
    {
        // order matters, otherwise use-after-free -sh
        tracker = nullptr;
        libs = SelectedLibraries();
    }
};
