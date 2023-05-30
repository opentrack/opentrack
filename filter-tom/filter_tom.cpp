/* Copyright (c) 2023 Tom Brazier <tom_github@firstsolo.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "filter_tom.h"
#include "compat/math-imports.hpp"
#include "compat/macros.h"

#include "api/plugin-api.hpp"
#include "opentrack/defs.hpp"

#include <algorithm>

tom::tom()
{
}

void tom::filter(const double* input, double* output)
{
    // order of axes: x, y, z, yaw, pitch, roll
    if (unlikely(first_run))
    {
        first_run = false;
        t.start();

        std::fill(speeds, speeds + 6, 0.0);
        std::copy(input, input + 6, filtered_output);
    }
    else
    {
        const double dt = t.elapsed_seconds();
        t.start();

        for (int i = 0; i < 6; i++)
        {
            double speed = (input[i] - last_input[i]) / dt;
            speeds[i] += dt * (double)s.responsiveness[i] * (speed - speeds[i]);
            filtered_output[i] += dt * (double)s.responsiveness[i] * min(1.0, abs(speeds[i]) / (double)s.drift_speeds[i]) * (input[i] - filtered_output[i]);
        }
      }

    std::copy(input, input + 6, last_input);
    std::copy(filtered_output, filtered_output + 6, output);
}

OPENTRACK_DECLARE_FILTER(tom, dialog_tom, tomDll)
