/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_hydra.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

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

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
