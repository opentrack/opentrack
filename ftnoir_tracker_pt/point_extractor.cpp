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
    const int W = frame.cols;
	const int H = frame.rows; 

	// clear old points
	points.clear();

	// convert to grayscale
	Mat frame_grey;
	cvtColor(frame, frame_grey, COLOR_BGR2GRAY);

	// convert to binary
	Mat frame_bin;
	threshold(frame_grey, frame_bin, threshold_val, 255, THRESH_BINARY);

	unsigned int region_size_min = 3.14*min_size*min_size;
    unsigned int region_size_max = 3.14*max_size*max_size;

    int blob_index = 1;
    for (int y=0; y<H; y++)
	{
        for (int x=0; x<W; x++)
		{
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
					float val = frame_grey.at<unsigned char>(i,j);
					val = float(val - threshold_val)/(256 - threshold_val);
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
			points.push_back(c);
            
			if (blob_index >= 255) break;
        }
		if (blob_index >= 255) break;
    }
	
	// draw output image
	if (draw_output) {	
		frame.setTo(Scalar(255,0,0), frame_bin);
	}

	return points;
}
