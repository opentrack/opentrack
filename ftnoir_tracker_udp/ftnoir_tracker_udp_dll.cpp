#include "ftnoir_tracker_udp.h"
#include "facetracknoir/plugin-support.h"

void FTNoIR_TrackerDll::getFullName(QString *strToBeFilled)
{
    *strToBeFilled = "UDP";
}

void FTNoIR_TrackerDll::getShortName(QString *strToBeFilled)
{
    *strToBeFilled = "UDP";
}

void FTNoIR_TrackerDll::getDescription(QString *strToBeFilled)
{
    *strToBeFilled = "UDP";
}

void FTNoIR_TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/facetracknoir.png");
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
