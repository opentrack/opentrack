/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt_dll.h"

#include <QIcon>

//-----------------------------------------------------------------------------
void TrackerDll::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = "PointTracker 0.2";
}

void TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = "PointTracker";
}

void TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = "Tracks a 3-point model with know geometry like Freetrack / TrackIR";
}

void TrackerDll::getIcon(QIcon *icon)
{
	*icon = QIcon(":/Resources/icon.ico");
}


//-----------------------------------------------------------------------------
#pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerDllPtr __stdcall GetTrackerDll()
{
	return new TrackerDll;
}
