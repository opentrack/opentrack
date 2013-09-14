#include "ftnoir_protocol_wine.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

FTNoIR_ProtocolDll::FTNoIR_ProtocolDll() {
}

FTNoIR_ProtocolDll::~FTNoIR_ProtocolDll()
{

}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Protocol object.

// Export both decorated and undecorated names.
//   GetProtocolDll     - Undecorated name, which can be easily used with GetProcAddress
//						Win32 API function.
//   _GetProtocolDll@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetProtocolDll=_GetProtocolDll@0")

extern "C" FTNOIR_PROTOCOL_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_ProtocolDll;
}
