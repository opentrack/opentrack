/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTEXTRACTOR_H
#define POINTEXTRACTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc_c.h>

// ----------------------------------------------------------------------------
// Extracts points from an opencv image
class PointExtractor
{
public:	
	// extracts points from frame and draws some processing info into frame, if draw_output is set
	// dt: time since last call in seconds
	// WARNING: returned reference is valid as long as object
	const std::vector<cv::Vec2f>& extract_points(cv::Mat frame, float dt, bool draw_output);
	const std::vector<cv::Vec2f>& get_points() { return points; }
	PointExtractor();

	int threshold_val;
	int threshold_secondary_val;
	int min_size, max_size;	

protected:
	std::vector<cv::Vec2f> points;
	cv::Mat frame_last;
};

#endif //POINTEXTRACTOR_H
