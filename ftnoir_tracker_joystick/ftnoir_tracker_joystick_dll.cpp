#include "ftnoir_tracker_joystick.h"
#include "facetracknoir/plugin-support.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_TrackerDll;
}
