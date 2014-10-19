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


FTNoIR_Filter::FTNoIR_Filter() :
    first_run(true),
    // Currently facetracknoir/tracker.cpp updates every dt=3ms. All
    // filter alpha values are calculated as alpha=dt/(dt+RC) and
    // need to be updated when tracker.cpp changes.
    // TODO(abo): Change this to use a dynamic dt using a timer.
    // Deltas are smoothed over the last 1/60sec (16ms).
    delta_alpha(0.003/(0.003 + 0.016)),
    // Noise is smoothed over the last 60sec.
    noise_alpha(0.003/(0.003 + 60.0))
{
}

void FTNoIR_Filter::receiveSettings()
{
    s.b->reload();
}

void FTNoIR_Filter::filter(const double *target_camera_position,
                                       double *new_camera_position)
{
    double new_delta, new_noise, norm_noise;
    double smoothing, RC, alpha;

    //On the first run, initialize filter states to target intput.
    if (first_run==true) {
        for (int i=0;i<6;i++) {
            output[i] = target_camera_position[i];
            delta[i] = 0.0;
            noise[i] = 0.0;
        }
        first_run=false;
    }

    // Calculate the new camera position.
    for (int i=0;i<6;i++) {
        // Calculate the current and smoothed delta.
        new_delta = target_camera_position[i]-output[i];
        delta[i] = delta_alpha*new_delta + (1.0-delta_alpha)*delta[i];
        // Calculate the current and smoothed noise variance.
        new_noise = delta[i]*delta[i];
        noise[i] = noise_alpha*new_noise + (1.0-noise_alpha)*noise[i];
        // Normalise the noise between 0->1 for 0->9 variances (0->3 stddevs).
        norm_noise = std::min<double>(new_noise/(9.0*noise[i]), 1.0);
        // Calculate the smoothing 0.0->1.0 from the normalized noise.
        // TODO(abo): change kSmoothingScaleCurve to a float where 1.0 is sqrt(norm_noise).
        smoothing = 1.0 - pow(norm_noise, s.kSmoothingScaleCurve/20.0);
        // Currently min/max smoothing are ints 0->100. We want 0.0->3.0 seconds.
        // TODO(abo): Change kMinSmoothing, kMaxSmoothing to floats 0.0->3.0 seconds RC.
        RC = 3.0*(s.kMinSmoothing + smoothing*(s.kMaxSmoothing - s.kMinSmoothing))/100.0;
        // TODO(abo): Change this to use a dynamic dt using a timer.
        alpha = 0.003/(0.003 + RC);
        // Calculate the new output position.
        output[i] = alpha*target_camera_position[i] + (1.0-alpha)*output[i];
        new_camera_position[i] = output[i];
    }
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}
