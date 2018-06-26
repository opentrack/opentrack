/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#include "export.hpp"
#include "time.hpp"

#include <ctime>
#include <type_traits>

struct OTR_COMPAT_EXPORT Timer final
{
    using time_type = time_t;

    Timer();
    void start();

    template<typename t>
    t elapsed() const
    {
        using namespace time_units;
        return time_cast<t>(ns(elapsed_nsecs()));
    }

    time_type elapsed_nsecs() const;
    double elapsed_usecs() const;
    double elapsed_ms() const;
    double elapsed_seconds() const;
private:
    struct timespec state {};
    static void gettime(struct timespec* state);
    time_type conv_nsecs(const struct timespec& cur) const;
    using ns = time_units::ns;
};
