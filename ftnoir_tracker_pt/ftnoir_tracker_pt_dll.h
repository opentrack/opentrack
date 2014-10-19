/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#if defined(OPENTRACK_API)
#   include "facetracknoir/plugin-api.hpp"
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
    QString name() { return QString("PointTracker 1.1"); }
    QIcon icon() { return QIcon(":/Resources/Logo_IR.png"); }
};
