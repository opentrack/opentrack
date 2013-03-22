#ifndef FTNOIR_FILTER_BASE_H
#define FTNOIR_FILTER_BASE_H

#include "ftnoir_filter_base_global.h"
#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"
#include <QString>
#include <QList>
#include <QFile>
#include <QCoreApplication>
#include <QMessageBox>
#include <QSettings>

////////////////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
#   define EXTERN_C     extern "C"
#else
#   define EXTERN_C
#endif // __cplusplus

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct IFilter
{
	virtual ~IFilter() {}
	virtual void Initialize() = 0;
	virtual void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget) = 0;
};

typedef IFilter* IFilterPtr;
//typedef IFilter *(__stdcall *importGetFilter)(void);

// Factory function that creates instances of the Filter object.
EXTERN_C
FTNOIR_FILTER_BASE_EXPORT
IFilterPtr
__stdcall
GetFilter(void);

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct IFilterDialog
{
	virtual ~IFilterDialog() {}
	virtual void Initialize(QWidget *parent, IFilterPtr ptr) = 0;
};

typedef IFilterDialog* IFilterDialogPtr;


// Factory function that creates instances of the Filter object.
EXTERN_C
FTNOIR_FILTER_BASE_EXPORT
IFilterDialogPtr
__stdcall
GetFilterDialog(void);

////////////////////////////////////////////////////////////////////////////////
// COM-Like abstract interface.
// This interface doesn't require __declspec(dllexport/dllimport) specifier.
// Method calls are dispatched via virtual table.
// Any C++ compiler can use it.
// Instances are obtained via factory function.
struct IFilterDll
{
	virtual ~IFilterDll() {}

	virtual void getFullName(QString *strToBeFilled) = 0;
	virtual void getShortName(QString *strToBeFilled) = 0;
	virtual void getDescription(QString *strToBeFilled) = 0;
	virtual void getIcon(QIcon *icon) = 0;
};

typedef IFilterDll* IFilterDllPtr;

// Factory function that creates instances of the Filter object.
EXTERN_C
FTNOIR_FILTER_BASE_EXPORT
IFilterDllPtr
__stdcall
GetFilterDll(void);


#endif // FTNOIR_FILTER_BASE_H
