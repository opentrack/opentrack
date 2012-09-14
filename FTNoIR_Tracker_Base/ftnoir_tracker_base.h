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
/*
	Modifications (last one on top):
		20120009 - WVR: Removed AutoClosePtr (seemed like it didn't work OK)
		20110415 - WVR: Added overloaded operator - and -=
*/
#ifndef FTNOIR_TRACKER_BASE_H
#define FTNOIR_TRACKER_BASE_H

#include "ftnoir_tracker_base_global.h"
#include "ftnoir_tracker_types.h"
#include <QtGui/QWidget>
#include <QtGui/QFrame>

// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITracker
{
    virtual ~ITracker() {};
//	virtual void Release() = 0;									// Member required to enable Auto-remove
	virtual void Initialize( QFrame *videoframe ) = 0;
	virtual void StartTracker( HWND parent_window ) = 0;
	virtual void StopTracker(bool exit) = 0;
	virtual bool GiveHeadPoseData(THeadPoseData *data) = 0;

	virtual bool notifyZeroed() {
		return false;
	}
	virtual void refreshVideo() {}
};

// Handle type. In C++ language the interface type is used.
typedef ITracker* TRACKERHANDLE;

////////////////////////////////////////////////////////////////////////////////
// 
#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus

// Factory function that creates instances of the Tracker object.
EXTERN_C
FTNOIR_TRACKER_BASE_EXPORT
TRACKERHANDLE
__stdcall
GetTracker(
    void);


// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITrackerDialog
{
	virtual void Initialize(QWidget *parent) = 0;
	virtual void registerTracker(ITracker *tracker) = 0;
	virtual void unRegisterTracker() = 0;
};

// Handle type. In C++ language the iterface type is used.
typedef ITrackerDialog* TRACKERDIALOGHANDLE;

// Factory function that creates instances of the Tracker object.
EXTERN_C
FTNOIR_TRACKER_BASE_EXPORT
TRACKERDIALOGHANDLE
__stdcall
GetTrackerDialog(void);

// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITrackerDll
{
//    virtual void Release() = 0;									// Member required to enable Auto-remove
	virtual void Initialize() = 0;

	virtual void getFullName(QString *strToBeFilled) = 0;
	virtual void getShortName(QString *strToBeFilled) = 0;
	virtual void getDescription(QString *strToBeFilled) = 0;
	virtual void getIcon(QIcon *icon) = 0;
};

// Handle type. In C++ language the interface type is used.
typedef ITrackerDll* TRACKERDLLHANDLE;

// Factory function that creates instances of the Tracker object.
EXTERN_C
FTNOIR_TRACKER_BASE_EXPORT
TRACKERDLLHANDLE
__stdcall
GetTrackerDll(void);


#endif // FTNOIR_TRACKER_BASE_H
