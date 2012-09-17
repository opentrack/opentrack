/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"

#include <QDebug>

using namespace cv;
using namespace std;

// ----------------------------------------------------------------------------
const vector<Vec2f>& PointExtractor::extract_points(Mat frame, float dt, bool draw_output)
{
	// convert to grayscale
	Mat frame_bw;
	cvtColor(frame, frame_bw, CV_RGB2GRAY);

	// convert to binary
	threshold(frame_bw, frame_bw, threshold_val, 255, THRESH_BINARY);
	erode(frame_bw, frame_bw, Mat(), Point(-1,-1), min_size);

	// find contours
	vector< vector<Point> > contours;
	findContours(frame_bw.clone(), contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

	// extract points
	// TODO: use proximity to old points for classification
	float r;
	Vec2f c;
	Point2f dummy;
	points.clear();
	for (vector< vector<Point> >::iterator iter = contours.begin();
		 iter!= contours.end();
		 ++iter)
	{
		minEnclosingCircle(*iter, dummy, r);
		if (r > max_size - min_size) break;
		Moments m = moments(*iter);
		if (m.m00 == 0) break;
		// convert to centered camera coordinate system with y axis upwards
		c[0] =  (m.m10/m.m00 - frame.cols/2)/frame.cols;
		c[1] = -(m.m01/m.m00 - frame.rows/2)/frame.cols;
		points.push_back(c);
	}
	
	// draw output image
	if (draw_output)
	{	
		frame.setTo(Scalar(255,0,0), frame_bw);
	}

	return points;
}
