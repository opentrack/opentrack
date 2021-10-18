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

struct OTR_COMPAT_EXPORT Timer final
{
    Timer();
    void start();

    double elapsed_ms() const;
    double elapsed_seconds() const;

private:
    struct timespec state {};
    static void gettime(struct timespec* state);
    struct timespec get_delta() const;
    using ns = time_units::ns;
};
