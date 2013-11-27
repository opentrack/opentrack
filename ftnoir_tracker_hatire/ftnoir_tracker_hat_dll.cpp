#include "ftnoir_tracker_hat.h"
#include <QIcon>
#include <QDebug>

FTNoIR_TrackerDll::FTNoIR_TrackerDll() {
    trackerFullName = "Hatire Arduino";
	trackerShortName = "HAT";
    trackerDescription = "Hatire Arduino";
}

FTNoIR_TrackerDll::~FTNoIR_TrackerDll()
{
}

void FTNoIR_TrackerDll::getFullName(QString *strToBeFilled)
{
	*strToBeFilled = trackerFullName;
}

void FTNoIR_TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = trackerShortName;
}

void FTNoIR_TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = trackerDescription;
}

void FTNoIR_TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/hat.png");
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
