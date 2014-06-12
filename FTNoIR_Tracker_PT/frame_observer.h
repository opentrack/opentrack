/* Copyright (c) 2013 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FRAME_OBSERVER_H
#define FRAME_OBSERVER_H

#include <QMutex>
#include <opencv2/opencv.hpp>
#ifndef OPENTRACK_API
#   include <boost/shared_ptr.hpp>
#else
#   include "FTNoIR_Tracker_PT/boost-compat.h"
#endif
#include <set>

//-----------------------------------------------------------------------------
// Forward declarations
class FrameObserver;

//-----------------------------------------------------------------------------
// Provides means to copy frame and point information if it has observers
// Instantiate a FrameObserver to get the information
class FrameProvider
{
	friend class FrameObserver;
public:
	~FrameProvider();

protected:
	virtual bool get_frame_and_points(cv::Mat& frame, boost::shared_ptr< std::vector<cv::Vec2f> >& points) = 0;
	
	bool has_observers() const { QMutexLocker lock(&observer_mutex); return !frame_observers.empty(); }

private:
	mutable QMutex observer_mutex;
	void add_observer(FrameObserver* obs)    { QMutexLocker lock(&observer_mutex); frame_observers.insert(obs); }
	void remove_observer(FrameObserver* obs) { QMutexLocker lock(&observer_mutex); frame_observers.erase(obs); }
	std::set<FrameObserver*> frame_observers;
};

//-----------------------------------------------------------------------------
// Used to get frame and point information from MutexedFrameProvider
// Destroy instance if not interested anymore since a living 
// FrameObserver instance causes MutexedFrameProvider to provide the information, 
// potentially reducing its performance
class FrameObserver
{
public:
	FrameObserver(FrameProvider* provider) : provider(provider) { 
		provider->add_observer(this); 
	}

	~FrameObserver() { 
		if (provider) provider->remove_observer(this); 
	}

	bool get_frame_and_points(cv::Mat& frame, boost::shared_ptr< std::vector<cv::Vec2f> >& points) {
		return provider ? provider->get_frame_and_points(frame, points) : false;
	}

	void on_frame_provider_destroy() { 
		provider = NULL; 
	}

protected:
	FrameProvider* provider;

private:
	FrameObserver(const FrameObserver&);
};

#endif //FRAME_OBSERVER_H
