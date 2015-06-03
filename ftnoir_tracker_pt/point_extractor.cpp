/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include <QDebug>
#include <algorithm>
#include <iostream>
#include <boost/format.hpp>

using namespace cv;
using namespace std;


struct Blob
{
	Rect region;
	int index;
	unsigned long int weight;
	Vec2d com;
};


/*
http://en.wikipedia.org/wiki/Mean-shift
*/
static Vec2d MeanShiftIteration(const Mat &frame_gray, Mat &display, const Vec2d &current_center, double filter_width)
{
	const int W = frame_gray.cols;
	const int H = frame_gray.rows; 
	const double s = 1.0/filter_width;
	
	auto EvalFilter = [=](int i, int j) -> double
	{
		double dx = (j - current_center[0])*s;
		double dy = (i - current_center[1])*s;
		double f = max<double>(0.0, 1.0 - dx*dx - dy*dy); // i'd use a gaussian but this is faster!
		return f*f;
	};

	double m = 0;
	Vec2d com(0.0, 0.0);
	for (int i=0; i < H; i++)
	{
		for (int j=0; j < W; j++)
		{
			double val = frame_gray.at<float>(i,j);
			val *= EvalFilter(i, j);
			m      += val;
			com[0] += j * val;
			com[1] += i * val; 
		}
	}
	com *= 1.0/m;
	return com;
}


PointExtractor::PointExtractor(){
}
// ----------------------------------------------------------------------------
const vector<Vec2f>& PointExtractor::extract_points(Mat& frame)
{
	const int W = frame.cols;
	const int H = frame.rows; 
	unsigned int region_size_min = 3.14*s.min_point_size*s.min_point_size/4.0;
	unsigned int region_size_max = 3.14*s.max_point_size*s.max_point_size/4.0;
	
    if (frame_last.cols != W || frame_last.rows != H)
    {
        frame_last = cv::Mat();
    }

	// clear old points
	points.clear();

	Mat frame_gray;
    cvtColor(frame, frame_gray, CV_RGB2GRAY);
		
	Mat frame_thresholded;
	threshold(frame_gray, frame_thresholded, s.threshold, 255, THRESH_BINARY);

	Mat frame_bin = frame_thresholded.clone();
	
	std::vector<Blob> blobs;
	int blob_index = 1;
	for (int y=0; y<H; y++)
	{
		if (blob_index >= 255) break;
		for (int x=0; x<W; x++)
		{
			if (blob_index >= 255) break;

			// find connected components with floodfill
			if (frame_bin.at<unsigned char>(y,x) != 255) continue;
			Rect rect;

			blob_index++;
			floodFill(frame_bin, Point(x,y), Scalar(blob_index), &rect, Scalar(0), Scalar(0), FLOODFILL_FIXED_RANGE);

			Blob b;
			b.index  = blob_index;
			b.region = rect;
			b.weight = 0;
			b.com    = Vec2d(0.0, 0.0);
			
			// calculate the size of the connected component
			unsigned int region_size = 0;
			for (int i=rect.y; i < (rect.y+rect.height); i++)
			{
				for (int j=rect.x; j < (rect.x+rect.width); j++)
				{
					if (frame_bin.at<unsigned char>(i,j) != blob_index) continue;
					region_size++;
					double val = frame_gray.at<unsigned char>(i,j);
					b.weight += val;
					b.com[0] += j * val;
					b.com[1] += i * val;
			 	}
			}
			b.com *= 1.0/b.weight;
			
			if (region_size < region_size_min || region_size > region_size_max) continue; 
			
			blobs.push_back(b);
		}
	}
	
	Mat frame_display = frame_bin; // reuse mem
	frame_gray.copyTo(frame_display);
	
	if (blobs.size() > 3) // only consider largest 3 blobs
	{
		std::stable_sort(blobs.begin(), blobs.end(), [](const Blob &a, const Blob &b) -> bool { a.weight > b.weight; });
		blobs.resize(3);
	}
	
	const int region_extension = s.threshold_secondary / 10;
	
	for (auto it = blobs.begin(); it != blobs.end(); ++it)
	{
		Rect rect = it->region;
		rect = rect + Size2i(region_extension*2, region_extension*2) - Point(region_extension, region_extension);
		int sz = min(rect.width, rect.height)/2 | 1;
		rect &= Rect(0, 0, W, H);

		Mat frame_display_roi (frame_display, rect);
		Mat frame_roi; Mat(frame_gray, rect).convertTo(frame_roi, CV_32FC1, 1.0/256.0);

		//GaussianBlur(frame_roi, frame_roi, Size2i(sz, sz), 0); //Size2i(region_extension|1,region_extension|1), 0);
		
		int fs = min(rect.width, rect.height)*0.4;  // the radius of the filter function
		Vec2d com = it->com - Vec2d(rect.x, rect.y);
		for (int iter = 0; iter < 20; ++iter)
		{
			Vec2d com_new = MeanShiftIteration(frame_roi, frame_display_roi, com, fs);
			Vec2d delta = com_new - com; 
			//std::cerr << boost::format("shift blob %i, it %i, is %s @ point %s\n") % (it - blobs.begin()) % iter % delta % com;
			com = com_new;
			if (delta.dot(delta) < 1.0e-4) break;
		}
		
		// convert to centered camera coordinate system with y axis upwards 
		Vec2f c;
		c[0] =  (com[0] + rect.x - W/2)/W;
		c[1] = -(com[1] + rect.y - H/2)/W;
		points.push_back(c);
		
		frame_roi.convertTo(frame_display_roi, CV_8UC1, 255.0);
		rectangle(frame_display, rect, Scalar(255), 2);
	}
	
	// draw output image
    vector<Mat> channels;
	Mat frame_display_1 = frame_display;
	Mat frame_display_2 = frame_display.clone();
	    frame_display_2.setTo(0, frame_thresholded);
	channels.push_back(frame_display_1);
	channels.push_back(frame_display_2);
	channels.push_back(frame_display_2);
    merge(channels, frame);

	return points;
}
