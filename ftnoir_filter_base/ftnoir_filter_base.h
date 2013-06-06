#ifndef FTNOIR_FILTER_BASE_H
#define FTNOIR_FILTER_BASE_H

#include "ftnoir_filter_base_global.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
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
    virtual void FilterHeadPoseData(double *current_camera_position, double *target_camera_position, double *new_camera_position, double *last_post_filter) = 0;
};

// Factory function that creates instances of the Filter object.

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

struct IFilterDialog
{
    virtual ~IFilterDialog() {}
    virtual void Initialize(QWidget *parent, IFilter* ptr) = 0;

    virtual void getFullName(QString *strToBeFilled) {};
    virtual void getShortName(QString *strToBeFilled) {};
    virtual void getDescription(QString *strToBeFilled) {};
    virtual void getIcon(QIcon *icon) {};
};

#endif // FTNOIR_FILTER_BASE_H
