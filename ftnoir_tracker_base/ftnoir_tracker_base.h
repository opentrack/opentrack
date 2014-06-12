/********************************************************************************
* FaceTrackNoIR		This program is a private project of the some enthusiastic	*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2010	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
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
* This class implements a tracker-base											*
*********************************************************************************/
#ifndef FTNOIR_TRACKER_BASE_H
#define FTNOIR_TRACKER_BASE_H

#include "ftnoir_tracker_base_global.h"
#include "ftnoir_tracker_types.h"
#include <QWidget>
#include <QFrame>
#include <QWaitCondition>
#include <QMutex>
#include <QFrame>

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITracker
{
    virtual ~ITracker() = 0;
    virtual void StartTracker( QFrame* frame ) = 0;
    virtual void GetHeadPoseData(double *data) = 0;
    virtual int preferredHz() { return 200; }
};

inline ITracker::~ITracker() { }

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITrackerDialog
{
    virtual ~ITrackerDialog() {}
    virtual void registerTracker(ITracker *tracker) = 0;
	virtual void unRegisterTracker() = 0;
};

#endif // FTNOIR_TRACKER_BASE_H
