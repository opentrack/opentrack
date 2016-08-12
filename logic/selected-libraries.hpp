/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "api/plugin-support.hpp"
#include <QFrame>

#include "export.hpp"

struct OPENTRACK_LOGIC_EXPORT SelectedLibraries
{
    using dylibptr = mem<dylib>;
    mem<ITracker> pTracker;
    mem<IFilter> pFilter;
    mem<IProtocol> pProtocol;
    SelectedLibraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f);
    SelectedLibraries() : pTracker(nullptr), pFilter(nullptr), pProtocol(nullptr), correct(false) {}
    bool correct;
};
