#pragma once

#include <QString>
#include <QFrame>

struct Metadata
{
    Metadata() {}
    virtual ~Metadata() {}

    virtual void getFullName(QString *strToBeFilled) = 0;
    virtual void getShortName(QString *strToBeFilled) = 0;
    virtual void getDescription(QString *strToBeFilled) = 0;
    virtual void getIcon(QIcon *icon) = 0;
};

// XXX TODO get rid of QString/QFrame to fix ABI woes
// will lead plugins from different C++ runtimes working -sh 20141004

// XXX TODO make virtual public the mess -sh 20141004

struct IFilter
{
    virtual ~IFilter() = 0;
    virtual void FilterHeadPoseData(const double *target_camera_position, double *new_camera_position) = 0;
};
inline IFilter::~IFilter() {}

struct IFilterDialog
{
    virtual ~IFilterDialog() = 0;
    virtual void registerFilter(IFilter* tracker) = 0;
    virtual void unregisterFilter() = 0;
};
inline IFilterDialog::~IFilterDialog() {}

struct IProtocol
{
    virtual ~IProtocol() = 0;
    virtual bool checkServerInstallationOK() = 0;
    virtual void sendHeadposeToGame( const double* headpose ) = 0;
    virtual QString getGameName() = 0;
};
inline IProtocol::~IProtocol() {}

struct IProtocolDialog
{
    virtual ~IProtocolDialog() = 0;
    virtual void registerProtocol(IProtocol *protocol) = 0;
    virtual void unRegisterProtocol() = 0;
};
inline IProtocolDialog::~IProtocolDialog() {}

struct ITracker
{
    virtual ~ITracker() = 0;
    virtual void StartTracker( QFrame* frame ) = 0;
    virtual void GetHeadPoseData(double *data) = 0;
};
inline ITracker::~ITracker() {}

struct ITrackerDialog
{
    virtual ~ITrackerDialog() = 0;
    virtual void registerTracker(ITracker *tracker) = 0;
    virtual void unRegisterTracker() = 0;
};
inline ITrackerDialog::~ITrackerDialog() {}
