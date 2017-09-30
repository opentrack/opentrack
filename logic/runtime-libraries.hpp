/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "api/plugin-support.hpp"
#include "export.hpp"

#include <array>
#include <functional>

#include <QFrame>

struct runtime_event_handler
{
    using ext_event_ordinal = IExtension::event_ordinal;
    using ext = std::shared_ptr<IExtension>;

    enum : unsigned { ext_max_events = 64 };
    using ext_list = std::array<ext, ext_max_events>;

    std::array<ext_list, ext_event_ordinal::event_count> extension_events;

    void run_events(ext_event_ordinal k, Pose& pose);
};

struct OTR_LOGIC_EXPORT runtime_libraries final : runtime_event_handler
{
    using dylibptr = std::shared_ptr<dylib>;

    std::shared_ptr<ITracker> pTracker;
    std::shared_ptr<IFilter> pFilter;
    std::shared_ptr<IProtocol> pProtocol;

    runtime_libraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f);
    runtime_libraries() : pTracker(nullptr), pFilter(nullptr), pProtocol(nullptr), correct(false) {}

    bool correct;
};
