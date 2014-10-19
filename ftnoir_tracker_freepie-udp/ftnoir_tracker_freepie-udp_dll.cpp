#include "ftnoir_tracker_freepie-udp.h"
#include "facetracknoir/plugin-support.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new TrackerMeta;
}
