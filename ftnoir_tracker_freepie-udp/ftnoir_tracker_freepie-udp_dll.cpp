#include "ftnoir_tracker_freepie-udp.h"
#include <QDebug>
#include "facetracknoir/plugin-support.h"

void TrackerMeta::getFullName(QString *strToBeFilled)
{
    *strToBeFilled = "FreePIE UDP";
}

void TrackerMeta::getShortName(QString *strToBeFilled)
{
    *strToBeFilled = "FreePIE";
}

void TrackerMeta::getDescription(QString *strToBeFilled)
{
    *strToBeFilled = "FreePIE UDP";
}

void TrackerMeta::getIcon(QIcon *icon)
{
    *icon = QIcon(":/glovepie.png");
}

extern "C" FTNOIR_TRACKER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
	return new TrackerMeta;
}
