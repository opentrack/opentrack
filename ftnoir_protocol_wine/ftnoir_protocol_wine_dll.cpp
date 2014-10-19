#include "ftnoir_protocol_wine.h"
#include "opentrack/plugin-api.hpp"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
