#ifndef FTNOIR_TRACKER_BASE_H
#define FTNOIR_TRACKER_BASE_H

#include "ftnoir_tracker_base_global.h"

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
    virtual void Release() = 0;
	virtual void Initialize() = 0;
	virtual void StartTracker() = 0;
	virtual void GiveHeadPoseData(THeadPoseData *data) = 0;
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

#endif // FTNOIR_TRACKER_BASE_H
