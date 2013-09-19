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

struct IFilter
{
	virtual ~IFilter() {}
    virtual void FilterHeadPoseData(const double *target_camera_position, double *new_camera_position, const double *last_post_filter) = 0;
    virtual void Initialize() = 0;
};

struct IFilterDialog
{
    virtual ~IFilterDialog() {}
    virtual void Initialize(QWidget *parent) = 0;
    virtual void registerFilter(IFilter* tracker) = 0;
    virtual void unregisterFilter() = 0;
};

#endif // FTNOIR_FILTER_BASE_H
