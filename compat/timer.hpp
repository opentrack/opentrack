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

class OTR_COMPAT_EXPORT Timer final
{
    struct timespec state;
    long long conv_nsecs(const struct timespec& cur) const;

    static void gettime(struct timespec* state);

    using ns = time_units::ns;
public:
    Timer();
    void start();

    template<typename t>
    t elapsed() const
    {
        using namespace time_units;
        return time_cast<t>(ns(elapsed_nsecs()));
    }

    template<typename t>
    bool is_elapsed(t&& time_value)
    {
        using namespace time_units;

        if (unlikely(elapsed<ns>() >= time_value))
        {
            start();
            return true;
        }
        return false;
    }

    long long elapsed_nsecs() const;
    double elapsed_usecs() const;
    double elapsed_ms() const;
    double elapsed_seconds() const;
};
