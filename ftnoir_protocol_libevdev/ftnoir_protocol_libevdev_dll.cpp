#include "ftnoir_protocol_libevdev.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
