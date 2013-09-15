/* Copyright (c) 2013 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "frame_observer.h"

//-----------------------------------------------------------------------------
FrameProvider::~FrameProvider()
{
	QMutexLocker lock(&observer_mutex);
    for (std::set<FrameObserver*>::iterator iter=frame_observers.begin(); iter!=frame_observers.end(); ++iter)
	{
		(*iter)->on_frame_provider_destroy();
	}
}
