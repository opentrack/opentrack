/* Copyright (c) 2013 Stanis³aw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "..\ftnoir_tracker_base\ftnoir_tracker_base.h"

//-----------------------------------------------------------------------------
class TrackerDll : public ITrackerDll
{
	// ITrackerDll interface
	void Initialize() {}
	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);
};