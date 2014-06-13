/********************************************************************************
* FaceTrackNoIR      This program is a private project of some enthusiastic     *
*                    gamers from Holland, who don't like to pay much for        *
*                    head-tracking.                                             *
*                                                                               *
* Copyright (C) 2012  Wim Vriend (Developing)                                   *
*                     Ron Hendriks (Researching and Testing)                    *
*                                                                               *
* Homepage                                                                      *
*                                                                               *
* This program is free software; you can redistribute it and/or modify it       *
* under the terms of the GNU General Public License as published by the         *
* Free Software Foundation; either version 3 of the License, or (at your        *
* option) any later version.                                                    *
*                                                                               *
* This program is distributed in the hope that it will be useful, but           *
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY    *
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for   *
* more details.                                                                 *
*                                                                               *
* You should have received a copy of the GNU General Public License along       *
* with this program; if not, see <http://www.gnu.org/licenses/>.                *
*                                                                               *
********************************************************************************/
#include "ftnoir_filter_ewma2.h"
#include "math.h"
#include <QDebug>
#include <QWidget>
#include "facetracknoir/global-settings.h"
#include <algorithm>
#include <QMutexLocker>

///////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
// This filter tries to adjust the amount of filtering to minimize lag when
// moving, and minimize noise when still. It uses the delta filtered over the
// last 3 frames (0.1secs) compared to the delta's average noise variance over
// the last 3600 frames (~2mins) to try and detect movement vs noise. As the
// delta increases from 0->3 stdevs of the noise, the filtering scales down
// from maxSmooth->minSmooth at a rate controlled by the powCurve setting.
//
// Written by Donovan Baarda
//
///////////////////////////////////////////////////////////////////////////////

FTNoIR_Filter::FTNoIR_Filter() :
    first_run(true),
    // Deltas are smoothed over the last 3 frames (0.1sec at 30fps).
    delta_smoothing(1.0/3.0),
    // Noise is smoothed over the last 3600 frames (~2mins at 30fps).
    noise_smoothing(1.0/3600.0)
{
}

void FTNoIR_Filter::receiveSettings()
{
    s.b->reload();
}

void FTNoIR_Filter::FilterHeadPoseData(const double *target_camera_position,
                                       double *new_camera_position)
{
    double new_delta, new_noise, norm_noise;
    double alpha;

    //On the first run, initialize to output=target and return.
    if (first_run==true) {
        for (int i=0;i<6;i++) {
            new_camera_position[i] = target_camera_position[i];
            current_camera_position[i] = target_camera_position[i];
            delta[i] = 0.0;
            noise[i] = 0.0;
        }
        first_run=false;
        return;
    }

    bool new_frame = false;

    for (int i = 0; i < 6; i++)
    {
        if (target_camera_position[i] != current_camera_position[i])
        {
            new_frame = true;
            break;
        }
    }

    if (!new_frame)
    {
        for (int i = 0; i < 6; i++)
            new_camera_position[i] = current_camera_position[i];
        return;
    }

    // Calculate the new camera position.
    for (int i=0;i<6;i++) {
        // Calculate the current and smoothed delta.
        new_delta = target_camera_position[i]-current_camera_position[i];
        delta[i] = delta_smoothing*new_delta + (1.0-delta_smoothing)*delta[i];
        // Calculate the current and smoothed noise variance.
        new_noise = delta[i]*delta[i];
        noise[i] = noise_smoothing*new_noise + (1.0-noise_smoothing)*noise[i];
        // Normalise the noise between 0->1 for 0->9 variances (0->3 stddevs).
        norm_noise = std::min<double>(new_noise/(9.0*noise[i]), 1.0);
        // Calculate the alpha from the normalized noise.
        // TODO(abo): change kSmoothingScaleCurve to a float where 1.0 is sqrt(norm_noise).
        alpha = 1.0/(s.kMinSmoothing+(1.0-pow(norm_noise,s.kSmoothingScaleCurve/20.0))*(s.kMaxSmoothing-s.kMinSmoothing));
        new_camera_position[i] = alpha*target_camera_position[i] + (1.0-alpha)*current_camera_position[i];
    }
    // Update the current camera position to the new position.
    for (int i = 0; i < 6; i++) {
        current_camera_position[i] = new_camera_position[i];
    }
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}
