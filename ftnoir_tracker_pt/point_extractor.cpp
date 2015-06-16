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


PointExtractor::PointExtractor(){
	//if (!AllocConsole()){}
	//else SetConsoleTitle("debug");
	//freopen("CON", "w", stdout);
	//freopen("CON", "w", stderr);
}
// ----------------------------------------------------------------------------
std::vector<Vec2f> PointExtractor::extract_points(Mat& frame)
{
	const int W = frame.cols;
	const int H = frame.rows; 

    if (frame_last.cols != W || frame_last.rows != H)
    {
        frame_last = cv::Mat();
    }

	// convert to grayscale
	Mat frame_gray;
    cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

	int secondary = s.threshold_secondary;
    int primary = s.threshold;
	
	// mask for everything that passes the threshold (or: the upper threshold of the hysteresis)
	Mat frame_bin;
	// only used if draw_output
	Mat frame_bin_copy;
	// mask for everything that passes
	Mat frame_bin_low;
	// mask for lower-threshold && combined result of last, needs to remain in scope until drawing, but is only used if secondary != 0
	Mat frame_last_and_low;

	if(secondary==0){
		threshold(frame_gray, frame_bin, primary, 255, THRESH_BINARY);
	}else{
		// we recombine a number of buffers, this might be slower than a single loop of per-pixel logic
		// but it might as well be faster if openCV makes good use of SIMD
		float t = primary;
		//float hyst = float(threshold_secondary_val)/512.;
		//threshold(frame_gray, frame_bin, (t + ((255.-t)*hyst)), 255, THRESH_BINARY);
		float hyst = float(primary)/(256.*8.);
		threshold(frame_gray, frame_bin, t, 255, THRESH_BINARY); 
		threshold(frame_gray, frame_bin_low,std::max(float(1), t - (t*hyst)), 255, THRESH_BINARY);

		frame_bin.copyTo(frame_bin_copy);
        if(frame_last.empty()){
			frame_bin.copyTo(frame_last);
		}else{
			// keep pixels from last if they are above lower threshold
			bitwise_and(frame_last, frame_bin_low, frame_last_and_low);
			// union of pixels >= higher threshold and pixels >= lower threshold
			bitwise_or(frame_bin, frame_last_and_low, frame_last);
			frame_last.copyTo(frame_bin);
		}
	}
    
    int min_size = s.min_point_size;
    int max_size = s.max_point_size;
    
	unsigned int region_size_min = 3.14*min_size*min_size/4.0;
	unsigned int region_size_max = 3.14*max_size*max_size/4.0;
    
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(frame_bin, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    
    // clear old points
	points.clear();
    
    for (auto& c : contours)
    {
        auto m = cv::moments(cv::Mat(c));
        const double area = m.m00;
        if (area == 0.)
            continue;
        cv::Vec2f pos(m.m10 / m.m00, m.m01 / m.m00);
        if (area < region_size_min || area > region_size_max)
            continue;
        pos[0] =  (pos[0] - W/2)/W;
        pos[1] = -(pos[1] - H/2)/W;
        
        points.push_back(pos);
    }
	
	// draw output image
    vector<Mat> channels;
    frame_bin.setTo(170, frame_bin);
    channels.push_back(frame_gray + frame_bin);
    channels.push_back(frame_gray - frame_bin);
    channels.push_back(frame_gray - frame_bin);
    merge(channels, frame);

	return points;
}
