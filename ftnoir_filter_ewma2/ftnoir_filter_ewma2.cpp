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
//#define LOG_OUTPUT

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FTNoIR_Filter::FTNoIR_Filter() : first_run(true), alpha_smoothing(0.02)
{
}

void FTNoIR_Filter::receiveSettings()
{
    s.b->reload();
}

void FTNoIR_Filter::FilterHeadPoseData(const double *target_camera_position,
                                       double *new_camera_position)
{
    double delta;
    double new_alpha;
    double scale[]={0.025f,0.025f,0.025f,6.0f,6.0f,6.0f};

    //On the first run, initialize to output=target and return.
    if (first_run==true) {
        for (int i=0;i<6;i++) {
            new_camera_position[i] = target_camera_position[i];
            current_camera_position[i] = target_camera_position[i];
            alpha[i] = 0.0f;
        }
        first_run=false;
        return;
    }
    
    for (int i=0;i<6;i++) {
        // Calculate the delta.
        delta=target_camera_position[i]-current_camera_position[i];
        // Normalise the delta.
        delta=std::min<double>(std::max<double>(fabs(delta)/scale[i],0.0),1.0);
        // Calculate the new alpha from the normalized delta.
        new_alpha=1.0/(s.kMinSmoothing+((1.0-pow(delta,s.kSmoothingScaleCurve))*(s.kMaxSmoothing-s.kMinSmoothing)));
        // Update the smoothed alpha.
        alpha[i]=(alpha_smoothing*new_alpha)+((1.0-alpha_smoothing)*alpha[i]);
    }

    // Use the same (largest) smoothed alpha for each channel
    //NB: larger alpha = *less* lag (opposite to what you'd expect)
    float largest_alpha=0.0f;
    for (int i=0;i<6;i++) {
        largest_alpha=std::min<double>(largest_alpha, alpha[i]);
    }

    // Calculate the new camera position.
    for (int i=0;i<6;i++) {
        new_camera_position[i]=(largest_alpha*target_camera_position[i])+((1.0-largest_alpha)*current_camera_position[i]);
        //new_camera_position[i]=(alpha[i]*target_camera_position[i])+((1.0f-alpha[i])*current_camera_position[i]);
    }

#ifdef LOG_OUTPUT
    // Use this for some debug-output to file...
    QFile data(QCoreApplication::applicationDirPath() + "\\EWMA_output.txt");
    if (data.open(QFile::WriteOnly | QFile::Append)) {
        QTextStream out(&data);
        out << "current:\t" << current_camera_position[0]
            << "\t" << current_camera_position[1]
            << "\t" << current_camera_position[2]
            << "\t" << current_camera_position[3]
            << "\t" << current_camera_position[4]
            << "\t" << current_camera_position[5] << '\n';
        out << "target:\t" << target_camera_position[0]
            << "\t" << target_camera_position[1]
            << "\t" << target_camera_position[2]
            << "\t" << target_camera_position[3]
            << "\t" << target_camera_position[4]
            << "\t" << target_camera_position[5] << '\n';
        out << "output:\t" << new_camera_position[0]
            << "\t" << new_camera_position[1]
            << "\t" << new_camera_position[2]
            << "\t" << new_camera_position[3]
            << "\t" << new_camera_position[4]
            << "\t" << new_camera_position[5] << '\n';
        out << "largest_alpha:\t" << largest_alpha << '\n';
    }
#endif

    // Update the current camera position to the new position.
    for (int i = 0; i < 6; i++) {
        current_camera_position[i] = new_camera_position[i];
    }
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter object.

// Export both decorated and undecorated names.
//   GetFilter     - Undecorated name, which can be easily used with GetProcAddress
//                   Win32 API function.
//   _GetFilter@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetFilter=_GetFilter@0")

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}
