#pragma once

#include "opentrack/main-settings.hpp"
#include "opentrack/plugin-support.h"
#include "opentrack/tracker.h"
#include "opentrack/shortcuts.h"

#include <QObject>
#include <QFrame>
#include <memory>

struct Work
{
    main_settings& s;
    SelectedLibraries libs;
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
#else
        QObject::connect(sc.get(), SIGNAL(center()), recv, SLOT(shortcutRecentered()));
        QObject::connect(sc.get(), SIGNAL(toggle()), recv, SLOT(shortcutToggled()));
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
