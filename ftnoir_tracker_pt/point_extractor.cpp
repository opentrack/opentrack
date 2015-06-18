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

	// clear old points
	points.clear();

    // convert to grayscale
    Mat frame_gray;
    cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);
    
    int primary = s.threshold;
	
    // mask for everything that passes the threshold (or: the upper threshold of the hysteresis)
    Mat frame_bin;
    
    threshold(frame_gray, frame_bin, primary, 255, THRESH_BINARY);
    
    int min_size = s.min_point_size;
    int max_size = s.max_point_size;
    
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

                                        val = frame_gray.at<unsigned char>(i,j);
                                        val = float(val - primary)/(256 - primary);
                                        val = val*val; // makes it more stable (less emphasis on low values, more on the peak)

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
    frame_bin.setTo(170, frame_bin);
    channels.push_back(frame_gray + frame_bin);
    channels.push_back(frame_gray - frame_bin);
    channels.push_back(frame_gray - frame_bin);
    merge(channels, frame);
    
	return points;
}
