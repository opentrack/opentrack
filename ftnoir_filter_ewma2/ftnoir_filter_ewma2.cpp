/*** Written by Donovan Baarda
*
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
#include <cmath>
#include <QDebug>
#include <QWidget>
#include "facetracknoir/plugin-support.h"
#include <algorithm>
#include <QMutexLocker>

FTNoIR_Filter::FTNoIR_Filter() :
    first_run(true),
    // Currently facetracknoir/tracker.cpp updates every dt=3ms. All
    // filter alpha values are calculated as alpha=dt/(dt + RC) and
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

void FTNoIR_Filter::FilterHeadPoseData(const double *target_camera_position,
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
