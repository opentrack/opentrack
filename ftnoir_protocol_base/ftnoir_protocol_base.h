#pragma once
#include "ftnoir_protocol_base_global.h"
#include "ftnoir_tracker_base/ftnoir_tracker_types.h"

struct IProtocol
{
    virtual ~IProtocol() = 0;
    virtual bool checkServerInstallationOK() = 0;
    virtual void sendHeadposeToGame( const double* headpose ) = 0;
    virtual QString getGameName() = 0;
};

inline IProtocol::~IProtocol() { }

struct IProtocolDialog
{
    virtual ~IProtocolDialog() {}
    virtual void registerProtocol(IProtocol *protocol) = 0;
    virtual void unRegisterProtocol() = 0;
};
