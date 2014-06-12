/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_hydra.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"

void FTNoIR_TrackerDll::getFullName(QString *strToBeFilled)
{
    *strToBeFilled = "Hydra";
}

void FTNoIR_TrackerDll::getShortName(QString *strToBeFilled)
{
    *strToBeFilled = "Hydra";
}

void FTNoIR_TrackerDll::getDescription(QString *strToBeFilled)
{
    *strToBeFilled = "Hydra";
}

void FTNoIR_TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/facetracknoir.png");
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTrackerDll     - Undecorated name, which can be easily used with GetProcAddress
//						Win32 API function.
//   _GetTrackerDll@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
