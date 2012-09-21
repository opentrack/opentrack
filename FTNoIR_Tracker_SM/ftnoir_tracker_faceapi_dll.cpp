/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
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
/*
	Modifications (last one on top):
		20120830 - WVR: The Dialog class was used to get general info on the DLL. This
						had a big disadvantage: the complete dialog was loaded, just to get
						some data and then it was deleted again (without ever showing the dialog).
						The TrackerDll class solves this.
						The functions to get the name(s) and icon were removed from the two other classes.
*/
#include "ftnoir_tracker_sm.h"
#include <QDebug>

FTNoIR_TrackerDll::FTNoIR_TrackerDll() {
	//populate the description strings
	trackerFullName = "faceAPI V3.2.6";
	trackerShortName = "faceAPI";
	trackerDescription = "SeeingMachines faceAPI V3.2.6";
}

FTNoIR_TrackerDll::~FTNoIR_TrackerDll()
{

}

void FTNoIR_TrackerDll::Initialize()
{
	return;
}

void FTNoIR_TrackerDll::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = trackerFullName;
};

void FTNoIR_TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = trackerShortName;
};

void FTNoIR_TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = trackerDescription;
};

void FTNoIR_TrackerDll::getIcon(QIcon *icon)
{
	*icon = QIcon(":/images/SeeingMachines.ico");
};

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTrackerDll     - Undecorated name, which can be easily used with GetProcAddress
//						Win32 API function.
//   _GetTrackerDll@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")

FTNOIR_TRACKER_BASE_EXPORT ITrackerDllPtr __stdcall GetTrackerDll()
{
	return new FTNoIR_TrackerDll;
}
