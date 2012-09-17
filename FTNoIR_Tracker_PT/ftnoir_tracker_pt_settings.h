/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef FTNOIR_TRACKER_PT_SETTINGS_H
#define FTNOIR_TRACKER_PT_SETTINGS_H

#include <opencv2/opencv.hpp>
#include "point_tracker.h"

//-----------------------------------------------------------------------------
struct TrackerSettings
{
	// camera
	int cam_index;
	float cam_f;

	// point extraction
	int threshold;
	int min_point_size;
	int max_point_size;

	// point tracking
	cv::Vec3f M01;
	cv::Vec3f M02;

	// head to model translation
	cv::Vec3f t_MH;

	int sleep_time;
	bool video_widget;

	void load_ini();
	void save_ini() const;
};

#endif //FTNOIR_TRACKER_PT_SETTINGS_H