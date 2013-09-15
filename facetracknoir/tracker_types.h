/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010 - 2012	Wim Vriend (Developing)								*
*							Ron Hendriks (Researching and Testing)				*
*																				*
* Homepage																		*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*********************************************************************************/
#ifndef __TRACKER_TYPES_H__
#define __TRACKER_TYPES_H__

#include "ftnoir_tracker_base/ftnoir_tracker_types.h"

struct T6DOF {
public:
    double axes[6];

    T6DOF() {
        for (int i = 0; i < 6; i++)
            axes[i] = 0;
    }
};

T6DOF operator-(const T6DOF& A, const T6DOF& B); // get new pose with respect to reference pose B
T6DOF operator+(const T6DOF& A, const T6DOF& B); // get new pose with respect to reference pose B^-1

#endif //__TRACKER_TYPES_H__
