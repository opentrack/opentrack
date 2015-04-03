/* Copyright (c) 2014 Donovan Baarda <abo@minkirri.apana.org.au>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_ewma2.h"
#include <cmath>
#include <QDebug>
#include <QWidget>
#include "opentrack/plugin-api.hpp"
#include <algorithm>
#include <QMutexLocker>

// Exponentially Weighted Moving Average (EWMA) Filter with dynamic smoothing.
//
// This filter tries to adjust the amount of filtering to minimize lag when
// moving, and minimize noise when still. It uses the delta filtered over the
// last 1/60sec (16ms) compared to the delta's average noise variance over
// the last 60sec to try and detect movement vs noise. As the delta increases
// from 0 to 3 stdevs of the noise, the filtering scales down from maxSmooth
// to minSmooth at a rate controlled by the powCurve setting.


FTNoIR_Filter::FTNoIR_Filter()
{
    reset();
}

void FTNoIR_Filter::receiveSettings()
{
    s.b->reload();
}

void FTNoIR_Filter::reset()
{
    timer.invalidate();
}

void FTNoIR_Filter::filter(const double *input, double *output)
{
    // Start the timer and initialise filter state if it's not running.
    if (!timer.isValid()) {
        timer.start();
        for (int i=0;i<6;i++) {
            last_output[i] = input[i];
            last_delta[i] = 0.0;
            last_noise[i] = 0.0;
        }
    }
    // Get the time in seconds since last run and restart the timer.
    auto dt = timer.restart() / 1000.0f;
    // Calculate delta_alpha and noise_alpha from dt.
    double delta_alpha = dt/(dt + delta_RC);
    double noise_alpha = dt/(dt + noise_RC);
    // Calculate the new camera position.
    for (int i=0;i<6;i++) {
        // Calculate the current and smoothed delta.
        double delta = input[i] - last_output[i];
        last_delta[i] = delta_alpha*delta + (1.0-delta_alpha)*last_delta[i];
        // Calculate the current and smoothed noise variance.
        double noise = last_delta[i]*last_delta[i];
        last_noise[i] = noise_alpha*noise + (1.0-noise_alpha)*last_noise[i];
        // Normalise the noise between 0->1 for 0->9 variances (0->3 stddevs).
        double norm_noise = std::min<double>(noise/(9.0*last_noise[i]), 1.0);
        // Calculate the smoothing 0.0->1.0 from the normalized noise.
        // TODO(abo): change kSmoothingScaleCurve to a float where 1.0 is sqrt(norm_noise).
        double smoothing = 1.0 - pow(norm_noise, s.kSmoothingScaleCurve/20.0);
        // Currently min/max smoothing are ints 0->100. We want 0.0->3.0 seconds.
        // TODO(abo): Change kMinSmoothing, kMaxSmoothing to floats 0.0->3.0 seconds RC.
        double RC = 3.0*(s.kMinSmoothing + smoothing*(s.kMaxSmoothing - s.kMinSmoothing))/100.0;
        // Calculate the dynamic alpha.
        double alpha = dt/(dt + RC);
        // Calculate the new output position.
        output[i] = last_output[i] = alpha*input[i] + (1.0-alpha)*last_output[i];
    }
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}
