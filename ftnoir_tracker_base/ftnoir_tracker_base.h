#pragma once
#include "ftnoir_tracker_base_global.h"
#include "ftnoir_tracker_types.h"
#include <QFrame>

struct ITracker
{
    virtual ~ITracker() = 0;
    virtual void StartTracker( QFrame* frame ) = 0;
    virtual void GetHeadPoseData(double *data) = 0;
    virtual int preferredHz() { return 200; }
};

inline ITracker::~ITracker() { }

struct ITrackerDialog
{
    virtual ~ITrackerDialog() {}
    virtual void registerTracker(ITracker *tracker) = 0;
	virtual void unRegisterTracker() = 0;
};
