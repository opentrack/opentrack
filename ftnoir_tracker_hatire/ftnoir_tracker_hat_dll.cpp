/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
* Homepage:			http://facetracknoir.sourceforge.net/home/default.htm		*
*																				*
* Copyright (C) 2012	FuraX49 (HAT Tracker plugins)	    	     			*
* Homepage:			http://hatire.sourceforge.net								*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/

#include "ftnoir_tracker_hat.h"
#include <QIcon>
#include <QDebug>

TrackerDll::TrackerDll() {
	//populate the description strings
    trackerFullName = "Hatire Arduino";
	trackerShortName = "HAT";
	trackerDescription = "FaceTrackNoIR HAT";
}

TrackerDll::~TrackerDll()
{

}

#ifndef OPENTRACK_API
void TrackerDll::Initialize()
{
	return;
}
#endif

#ifndef OPENTRACK_API
void TrackerDll::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = trackerFullName;
}

void TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = trackerShortName;
}

void TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = trackerDescription;
}

void TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/hat.png");
}
#else

QString TrackerDll::name()
{
    return trackerFullName;
}

QIcon TrackerDll::icon()
{
    return QIcon(":/images/hat.png");
}

#endif

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTrackerDll     - Undecorated name, which can be easily used with GetProcAddress
//						Win32 API function.
//   _GetTrackerDll@0  - Common name decoration for __stdcall functions in C language.

#ifdef OPENTRACK_API
#   include "opentrack/plugin-support.h"
extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
#else
#   pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")
FTNOIR_TRACKER_BASE_EXPORT ITrackerDllPtr __stdcall GetTrackerDll()
#endif
{
	return new TrackerDll;
}
