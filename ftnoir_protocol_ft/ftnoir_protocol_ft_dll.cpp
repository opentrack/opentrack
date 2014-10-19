#include "opentrack/plugin-api.hpp"
#include "ftnoir_protocol_ft/ftnoir_protocol_ft.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
