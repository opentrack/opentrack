/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#if defined(OPENTRACK_API)
#   include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#   include "facetracknoir/global-settings.h"
#else
#   include "../ftnoir_tracker_base/ftnoir_tracker_base.h"
#endif

//-----------------------------------------------------------------------------
class TrackerDll :
#if defined(OPENTRACK_API)
        public Metadata
#else
        public ITrackerDll
#endif
{
	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);
};
