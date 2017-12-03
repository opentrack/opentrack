/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "api/plugin-support.hpp"
#include "export.hpp"

#include <QFrame>

struct OTR_LOGIC_EXPORT runtime_libraries final
{
    using dylibptr = std::shared_ptr<dylib>;

    std::shared_ptr<ITracker> pTracker;
    std::shared_ptr<IFilter> pFilter;
    std::shared_ptr<IProtocol> pProtocol;

    runtime_libraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f);
    runtime_libraries() : pTracker(nullptr), pFilter(nullptr), pProtocol(nullptr), correct(false) {}

    bool correct = false;
};
