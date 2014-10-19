#include "ftnoir_tracker_freepie-udp.h"
#include "opentrack/plugin-api.hpp"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new TrackerMeta;
}
