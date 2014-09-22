#include "ftnoir_filter_accela.h"
#include "facetracknoir/plugin-support.h"

extern "C" OPENTRACK_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_FilterDll;
}
