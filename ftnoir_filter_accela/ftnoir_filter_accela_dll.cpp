#include "ftnoir_filter_accela.h"
#include "facetracknoir/plugin-support.h"

extern "C" FTNOIR_FILTER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_FilterDll;
}
