#ifndef FTNOIR_FILTER_BASE_H
#define FTNOIR_FILTER_BASE_H

#include "ftnoir_filter_base_global.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include <QString>
#include <QList>

// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct IFilter
{
    virtual void Release() = 0;
	virtual void Initialize() = 0;
	virtual void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget) = 0;

	virtual void getFilterFullName(QString *strToBeFilled) = 0;
	virtual void getFilterShortName(QString *strToBeFilled) = 0;
	virtual void getFilterDescription(QString *strToBeFilled) = 0;

	//parameter value get/set - returns true if successful, false if not
	virtual bool setParameterValue(const int index, const float newvalue) = 0;
};

// Handle type. In C++ language the iterface type is used.
typedef IFilter* FILTERHANDLE;

////////////////////////////////////////////////////////////////////////////////
// 
#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus

// Factory function that creates instances of the Filter object.
EXTERN_C
FTNOIR_FILTER_BASE_EXPORT
FILTERHANDLE
__stdcall
GetFilter(
    void);

#endif // FTNOIR_FILTER_BASE_H
