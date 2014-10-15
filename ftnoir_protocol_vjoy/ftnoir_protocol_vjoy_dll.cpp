#include "ftnoir_protocol_vjoy.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

FTNoIR_ProtocolDll::FTNoIR_ProtocolDll() {
}

FTNoIR_ProtocolDll::~FTNoIR_ProtocolDll()
{

}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
