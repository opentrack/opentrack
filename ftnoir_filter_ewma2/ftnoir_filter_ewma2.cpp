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
#include "facetracknoir/plugin-support.h"
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

void FTNoIR_Filter::FilterHeadPoseData(const double *input, double *output)
{
    double new_delta, new_noise;
    double smoothing, RC, alpha;
    
    QMutexLocker l(&state_mutex);

    //On the first run, initialize filter states to target intput.
    if (first_run==true) {
        for (int i=0;i<6;i++) {
            last_output[i] = input[i];
            delta[i] = 0.0;
            noise[i] = 0.0;
        }
        first_run=false;
    }

    const double s_min = s.kMinSmoothing;
    const double s_max = s.kMaxSmoothing + s_min;
    
    // Calculate the new camera position.
    for (int i=0;i<6;i++) {
        // Calculate the current and smoothed delta.
        new_delta = input[i]-last_output[i];
        delta[i] = delta_alpha*new_delta + (1.0-delta_alpha)*delta[i];
        // Calculate the current and smoothed noise variance.
        new_noise = delta[i]*delta[i];
        noise[i] = noise_alpha*new_noise + (1.0-noise_alpha)*noise[i];
        // Get the normalised stddevs between 0->1 for 0->3 stddevs.
        const double norm_stddevs = std::min(sqrt(new_noise/noise[i])/3.0, 1.0);
        // Calculate the smoothing 0.0->1.0 from the normalized noise.
        smoothing = 1.0 - norm_stddevs;
        RC = 3.0*(s_min + smoothing*(s_max - s_min))/100.0;
        // TODO(abo): Change this to use a dynamic dt using a timer.
        alpha = 0.003/(0.003 + RC);
        // Calculate the new output position.
        last_output[i] = alpha*input[i] + (1.0-alpha)*last_output[i];
        output[i] = last_output[i];
    }
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}
