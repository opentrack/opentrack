#include "ftnoir_filter_accela.h"
#include "opentrack/plugin-api.hpp"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_FilterDll;
}
