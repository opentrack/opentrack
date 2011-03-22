#ifndef FTNOIR_TRACKER_BASE_H
#define FTNOIR_TRACKER_BASE_H

#include "ftnoir_tracker_base_global.h"
#include <QtGui/QWidget>
#include <QtGui/QFrame>

//
// x,y,z position in centimetres, yaw, pitch and roll in degrees...
//
#pragma pack(push, 2)
struct THeadPoseData {
	double x, y, z, yaw, pitch, roll;
};
#pragma pack(pop)

// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct ITracker
{
    virtual void Release() = 0;									// Member required to enable Auto-remove
	virtual void Initialize( QFrame *videoframe ) = 0;
	virtual void StartTracker( HWND parent_window ) = 0;
	virtual void StopTracker(bool exit) = 0;
	virtual bool GiveHeadPoseData(THeadPoseData *data) = 0;
};

// Handle type. In C++ language the iterface type is used.
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
    virtual void Release() = 0;									// Member required to enable Auto-remove
	virtual void Initialize(QWidget *parent) = 0;
};

// Handle type. In C++ language the iterface type is used.
typedef ITrackerDialog* TRACKERDIALOGHANDLE;

// Factory function that creates instances of the Tracker object.
EXTERN_C
FTNOIR_TRACKER_BASE_EXPORT
TRACKERDIALOGHANDLE
__stdcall
GetTrackerDialog(void);


#endif // FTNOIR_TRACKER_BASE_H
