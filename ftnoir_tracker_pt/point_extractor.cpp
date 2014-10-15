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
const vector<Vec2f>& PointExtractor::extract_points(Mat& frame)
{
	const int W = frame.cols;
	const int H = frame.rows; 

    if (frame_last.cols != W || frame_last.rows != H)
    {
        frame_last = cv::Mat();
    }

	// clear old points
	points.clear();

	// convert to grayscale
	Mat frame_gray;
    cvtColor(frame, frame_gray, CV_RGB2GRAY);

	int secondary = threshold_secondary_val;
	
	// mask for everything that passes the threshold (or: the upper threshold of the hysteresis)
	Mat frame_bin;
	// only used if draw_output
	Mat frame_bin_copy;
	// mask for everything that passes
	Mat frame_bin_low;
	// mask for lower-threshold && combined result of last, needs to remain in scope until drawing, but is only used if secondary != 0
	Mat frame_last_and_low;

	if(secondary==0){
		threshold(frame_gray, frame_bin, threshold_val, 255, THRESH_BINARY);
	}else{
		// we recombine a number of buffers, this might be slower than a single loop of per-pixel logic
		// but it might as well be faster if openCV makes good use of SIMD
		float t = threshold_val;
		//float hyst = float(threshold_secondary_val)/512.;
		//threshold(frame_gray, frame_bin, (t + ((255.-t)*hyst)), 255, THRESH_BINARY);
		float hyst = float(threshold_secondary_val)/256.;
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
	unsigned int region_size_min = 3.14*min_size*min_size/4.0;
	unsigned int region_size_max = 3.14*max_size*max_size/4.0;

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

			floodFill(frame_bin, Point(x,y), Scalar(blob_index), &rect, Scalar(0), Scalar(0), FLOODFILL_FIXED_RANGE);
			blob_index++;

			// calculate the size of the connected component
			unsigned int region_size = 0;
			for (int i=rect.y; i < (rect.y+rect.height); i++)
			{
				for (int j=rect.x; j < (rect.x+rect.width); j++)
				{
					if (frame_bin.at<unsigned char>(i,j) != blob_index-1) continue;
					region_size++;
			 	}
			}
			
			if (region_size < region_size_min || region_size > region_size_max) continue; 

			// calculate the center of mass:
			// mx = (sum_ij j*f(frame_grey_ij)) / (sum_ij f(frame_grey_ij))
			// my = ...
			// f maps from [threshold,256] -> [0, 1], lower values are mapped to 0 
			float m = 0;
			float mx = 0;
			float my = 0;
			for (int i=rect.y; i < (rect.y+rect.height); i++)
			{
				for (int j=rect.x; j < (rect.x+rect.width); j++)
				{
					if (frame_bin.at<unsigned char>(i,j) != blob_index-1) continue;
					float val;

					if(secondary==0){
						val = frame_gray.at<unsigned char>(i,j);
						val = float(val - threshold_val)/(256 - threshold_val);
						val = val*val; // makes it more stable (less emphasis on low values, more on the peak)
					}else{
						//hysteresis point detection gets stability from ignoring pixel noise so we decidedly leave the actual pixel values out of the picture
						val = frame_last.at<unsigned char>(i,j) / 256.;
					}

					m  +=     val;
					mx += j * val;
					my += i * val; 
				}
			}

			// convert to centered camera coordinate system with y axis upwards 
			Vec2f c;
			c[0] =  (mx/m - W/2)/W;
			c[1] = -(my/m - H/2)/W;
			//qDebug()<<blob_index<<"  => "<<c[0]<<" "<<c[1];
			points.push_back(c);
		}
	}
	
	// draw output image
    vector<Mat> channels;
    if(secondary==0){
        frame_bin.setTo(170, frame_bin);
        channels.push_back(frame_gray + frame_bin);
        channels.push_back(frame_gray - frame_bin);
        channels.push_back(frame_gray - frame_bin);
    }else{
        frame_bin_copy.setTo(120, frame_bin_copy);
        frame_bin_low.setTo(90, frame_bin_low);
        channels.push_back(frame_gray + frame_bin_copy);
        channels.push_back(frame_gray + frame_last_and_low);
        channels.push_back(frame_gray + frame_bin_low);
        //channels.push_back(frame_gray + frame_bin);
    }
    merge(channels, frame);

	return points;
}
