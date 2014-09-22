#pragma once

#include <QtGlobal>
#include <QFrame>

#if defined(_WIN32)
#   define CALLING_CONVENTION __stdcall
#else
#   define CALLING_CONVENTION
#endif

enum Axis {
    TX = 0, TY, TZ, Yaw, Pitch, Roll
};

struct Metadata
{
    Metadata() {}
    virtual ~Metadata() {}

    virtual void getFullName(QString *strToBeFilled) = 0;
    virtual void getShortName(QString *strToBeFilled) = 0;
    virtual void getDescription(QString *strToBeFilled) = 0;
    virtual void getIcon(QIcon *icon) = 0;
};

struct IFilter
{
    virtual ~IFilter() = 0;
    virtual void FilterHeadPoseData(const double *target_camera_position, double *new_camera_position) = 0;
    virtual void reset() = 0;
};

inline IFilter::~IFilter() {}

struct IFilterDialog
{
    virtual ~IFilterDialog() {}
    virtual void registerFilter(IFilter* tracker) = 0;
    virtual void unregisterFilter() = 0;
};

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
    virtual ~IProtocolDialog() {}
    virtual void registerProtocol(IProtocol *protocol) = 0;
    virtual void unRegisterProtocol() = 0;
};

struct ITracker
{
    virtual ~ITracker() = 0;
    virtual void StartTracker( QFrame* frame ) = 0;
    virtual void GetHeadPoseData(double *data) = 0;
    virtual int preferredHz() { return 200; }
};

inline ITracker::~ITracker() {}

struct ITrackerDialog
{
    virtual ~ITrackerDialog() {}
    virtual void registerTracker(ITracker *tracker) = 0;
    virtual void unRegisterTracker() = 0;
};

#ifndef OPENTRACK_EXPORT
#   ifdef IN_OPENTRACK
#    if !defined(_MSC_VER)
#      define OPENTRACK_EXPORT __attribute__ ((visibility ("default"))) Q_DECL_EXPORT
#    else
#     error "MSVC support removed"
#   endif
#   else
#       error "Use only for exporting dynamic modules"
#   endif
#endif
