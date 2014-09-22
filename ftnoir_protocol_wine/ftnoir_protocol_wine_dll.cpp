#include "ftnoir_protocol_wine.h"
#include "facetracknoir/plugin-support.h"

FTNoIR_ProtocolDll::FTNoIR_ProtocolDll()
{
}

FTNoIR_ProtocolDll::~FTNoIR_ProtocolDll()
{
}

extern "C" OPENTRACK_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
