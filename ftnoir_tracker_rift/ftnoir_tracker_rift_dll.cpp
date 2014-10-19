/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
