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

struct BlobInfo
{
    BlobInfo() : m00(0), m10(0), m01(0) {}
    long m00;
    long m10;
    long m01;
};

// ----------------------------------------------------------------------------
const vector<Vec2f>& PointExtractor::extract_points(Mat frame, float dt, bool draw_output)
{
	/*
	// sensitivity test for tracker
	static int n = 0;
	if (points.size() == 3)
	{
		for (int i=0; i<3; ++i)
		{
			points[i][0] -= 1e-4*sin(n/100.0);
			points[i][1] -= 1e-4*cos(n/100.0);
		}
		++n;
		return points;
	}
	*/

	// convert to grayscale
	Mat frame_bw;
	cvtColor(frame, frame_bw, CV_RGB2GRAY);

	// convert to binary
	threshold(frame_bw, frame_bw, threshold_val, 255, THRESH_BINARY);
	//erode(frame_bw, frame_bw, Mat(), Point(-1,-1), min_size); //destroys information -> bad for subpixel accurarcy

	// find connected components...
	// extract blobs with floodfill
	vector<BlobInfo> blobs;
    int blob_count = 1;

    for (int y=0; y < frame_bw.rows; y++) {
        for (int x=0; x < frame_bw.cols; x++) {
			if (frame_bw.at<unsigned char>(y,x) != 255) continue;
            Rect rect;
            floodFill(frame_bw, Point(x,y), Scalar(blob_count), &rect, Scalar(0), Scalar(0), 4);
			BlobInfo blob;
            for (int i=rect.y; i < (rect.y+rect.height); i++) {
                for (int j=rect.x; j < (rect.x+rect.width); j++) {
                    if (frame_bw.at<unsigned char>(i,j) != blob_count) continue;
                    blob.m00++;
					blob.m01+=i;
					blob.m10+=j;
                }
            }
            blobs.push_back(blob);
            blob_count++;
			if (blob_count >= 255) break;
        }
		if (blob_count >= 255) break;
    }

	// extract points
	Vec2f c;
	points.clear();
	float m00_min = 3.14*min_size*min_size;
	float m00_max = 3.14*max_size*max_size;
	for (vector<BlobInfo>::iterator iter = blobs.begin();
		 iter!= blobs.end();
		 ++iter)
	{
		const BlobInfo& m = *iter;
		if (m.m00 < m00_min || m.m00 > m00_max) continue;
		// convert to centered camera coordinate system with y axis upwards
		c[0] =  (m.m10/float(m.m00) - frame.cols/2)/frame.cols;
		c[1] = -(m.m01/float(m.m00) - frame.rows/2)/frame.cols;
		points.push_back(c);
	}
	
	// draw output image
	if (draw_output)
	{	
		frame.setTo(Scalar(255,0,0), frame_bw);
	}

	return points;
}
