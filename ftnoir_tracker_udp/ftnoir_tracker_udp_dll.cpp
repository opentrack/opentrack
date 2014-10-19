#include "ftnoir_tracker_udp.h"
#include "opentrack/plugin-api.hpp"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
