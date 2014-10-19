#include "ftnoir_protocol_vjoy.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
