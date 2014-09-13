#pragma once
#include "ftnoir_filter_base_global.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"

struct IFilter
{
    virtual ~IFilter() = 0;
    virtual void FilterHeadPoseData(const double *target_camera_position, double *new_camera_position) = 0;
    virtual void reset() = 0;
};

inline IFilter::~IFilter() { }

struct IFilterDialog
{
    virtual ~IFilterDialog() {}
    virtual void registerFilter(IFilter* tracker) = 0;
    virtual void unregisterFilter() = 0;
};
