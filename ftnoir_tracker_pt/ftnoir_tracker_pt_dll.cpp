/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt_dll.h"
#include <QIcon>

#ifdef OPENTRACK_API
#   include "opentrack/plugin-api.hpp"
extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
#else
#   pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")
OPENTRACK_EXPORT ITrackerDllPtr __stdcall GetTrackerDll()
#endif
{
	return new TrackerDll;
}
