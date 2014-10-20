#pragma once

#include "opentrack/main-settings.hpp"
#include "opentrack/plugin-support.h"
#include "opentrack/tracker.h"
#include "opentrack/shortcuts.h"

#include <QObject>
#include <QFrame>
#include <memory>

struct Modules {
    Modules() :
        module_list(dylib::enum_libraries()),
        filter_modules(filter(dylib::Filter)),
        tracker_modules(filter(dylib::Tracker)),
        protocol_modules(filter(dylib::Protocol))
    {}
    QList<ptr<dylib>>& filters() { return filter_modules; }
    QList<ptr<dylib>>& trackers() { return tracker_modules; }
    QList<ptr<dylib>>& protocols() { return protocol_modules; }
private:
    QList<ptr<dylib>> module_list;
    QList<ptr<dylib>> filter_modules;
    QList<ptr<dylib>> tracker_modules;
    QList<ptr<dylib>> protocol_modules;
    
    QList<ptr<dylib>> filter(dylib::Type t)
    {
        QList<ptr<dylib>> ret;
        for (auto x : module_list)
            if (x->type == t)
                ret.push_back(x);
        return ret;
    }
};

struct Work
{
    main_settings& s;
    SelectedLibraries libs;
    ptr<Tracker> tracker;
    ptr<Shortcuts> sc;
    
    Work(main_settings& s, Mappings& m, SelectedLibraries& libs, QObject* recv) :
        s(s), libs(libs),
        tracker(std::make_shared<Tracker>(s, m, libs)),
        sc(std::make_shared<Shortcuts>())
    {
#ifndef _WIN32
        QObject::connect(&sc->keyCenter, SIGNAL(activated()), recv, SLOT(shortcutRecentered()));
        QObject::connect(&sc->keyToggle, SIGNAL(activated()), recv, SLOT(shortcutToggled()));
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