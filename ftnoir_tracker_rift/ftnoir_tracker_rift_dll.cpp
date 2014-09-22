/* Copyright: "i couldn't care less what anyone does with the 5 lines of code i wrote" - mm0zct */
#include "ftnoir_tracker_rift.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

FTNoIR_TrackerDll::FTNoIR_TrackerDll() {
	//populate the description strings
	trackerFullName = "Rift";
	trackerShortName = "Rift";
	trackerDescription = "Rift";
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
    *icon = QIcon(":/images/rift_tiny.png");
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new FTNoIR_TrackerDll;
}
