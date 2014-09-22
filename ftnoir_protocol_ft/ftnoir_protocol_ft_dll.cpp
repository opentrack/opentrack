#include "facetracknoir/plugin-support.h"
#include "ftnoir_protocol_ft/ftnoir_protocol_ft.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
