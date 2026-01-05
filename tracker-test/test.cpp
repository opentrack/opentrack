/* Copyright (c) 2014, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "test.h"
#include "api/plugin-api.hpp"
#include "compat/math-imports.hpp"

#include <cmath>
#include <QDebug>

static const double incr[3] =
{
    10, 5, 3
};

static const double max_values[3] = {
    180, 2, 3,
};

test_tracker::test_tracker() = default;
test_tracker::~test_tracker() = default;

module_status test_tracker::start_tracker(QFrame*)
{
    t.start();
    return {};
}

void test_tracker::data(double *data)
{
    const double dt = t.elapsed_seconds();
    t.start();

    for (int i = 0; i < 3; i++)
    {
        double last_ = last[i];
        double max = max_values[i] * 2;
        double incr_ = incr[i];
        double x = fmod(last_ + incr_ * dt, max);
        last[i] = x;
        if (x > max_values[i])
            x = -max + x;
        data[i+3] = x;
    }
}

OPENTRACK_DECLARE_TRACKER(test_tracker, test_dialog, test_metadata)
