#include "ftnoir_protocol_vjoy_sf.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
