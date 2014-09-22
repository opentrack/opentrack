#include "facetracknoir/plugin-support.h"
#include "ftnoir_protocol_ft/ftnoir_protocol_ft.h"

extern "C" OPENTRACK_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
