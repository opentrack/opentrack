#include "facetracknoir/plugin-support.h"
#include "ftnoir_protocol_ft/ftnoir_protocol_ft.h"

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
