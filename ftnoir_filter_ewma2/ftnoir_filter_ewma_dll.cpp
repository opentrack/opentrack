#include "ftnoir_filter_ewma2.h"
#include "opentrack/plugin-api.hpp"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new FTNoIR_FilterDll;
}
