#pragma once

#include "opentrack/plugin-support.hpp"
#include <QFrame>

struct SelectedLibraries {
    using dylibptr = mem<dylib>;
    mem<ITracker> pTracker;
    mem<IFilter> pFilter;
    mem<IProtocol> pProtocol;
    SelectedLibraries(QFrame* frame, mem<ITracker> t, dylibptr p, mem<IFilter> f);
    SelectedLibraries() : pTracker(nullptr), pFilter(nullptr), pProtocol(nullptr), correct(false) {}
    ~SelectedLibraries();
    bool correct;
};
