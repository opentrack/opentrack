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
    ptr<Tracker> tracker;
    ptr<Shortcuts> sc;
    
    Work(main_settings& s, Mappings& m, SelectedLibraries& libs, QObject* recv, WId handle) :
        s(s), libs(libs),
        tracker(std::make_shared<Tracker>(s, m, libs)),
        sc(std::make_shared<Shortcuts>(handle))
    {
#ifndef _WIN32
        QObject::connect(&sc->keyCenter, SIGNAL(activated()), recv, SLOT(shortcutRecentered()));
        QObject::connect(&sc->keyToggle, SIGNAL(activated()), recv, SLOT(shortcutToggled()));
#else
        QObject::connect(sc->keybindingWorker.get(), SIGNAL(center()), recv, SLOT(shortcutRecentered()));
        QObject::connect(sc->keybindingWorker.get(), SIGNAL(toggle()), recv, SLOT(shortcutToggled()));
#endif
        tracker->start();
    }
    
    ~Work()
    {
        // order matters, otherwise use-after-free -sh
        tracker = nullptr;
        libs = SelectedLibraries();
    }  
};
