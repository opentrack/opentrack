/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2017 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "cv-point-extractor.h"
#include "frame.hpp"

#include "cv/numeric.hpp"
#include "compat/math.hpp"
#include <opencv2/imgproc/types_c.h>

#include <cmath>
#include <algorithm>
#include <cinttypes>
#include <memory>

#include <QDebug>

using namespace numeric_types;



CvPointExtractor::CvPointExtractor(const QString& module_name) : s(module_name)
{
    
}


void CvPointExtractor::extract_points(const cv::Mat& frame, cv::Mat& aPreview, std::vector<vec2>& points, std::vector<vec2>& imagePoints)
{

    // Contours detection
    std::vector<std::vector<cv::Point> > contours;
    cv::findContours(frame, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    
    // Workout which countour are valid points
    std::vector<std::vector<cv::Point> > balls;
    std::vector<cv::Rect> ballsBox;
    for (size_t i = 0; i < contours.size(); i++)
    {
        cv::drawContours(aPreview, contours, i, CV_RGB(255, 0, 0), 2);

        cv::Rect bBox;
        bBox = cv::boundingRect(contours[i]);

        float ratio = (float)bBox.width / (float)bBox.height;
        if (ratio > 1.0f)
            ratio = 1.0f / ratio;

       
        // Searching for a bBox almost square
        float minArea = s.min_point_size*s.min_point_size;
        float maxArea = s.max_point_size*s.max_point_size;
        if (bBox.width >= s.min_point_size
            && bBox.height >= s.min_point_size
            && bBox.width <= s.max_point_size
            && bBox.height <= s.max_point_size
            && bBox.area() >= minArea
            && bBox.area() <= maxArea
            /*&& ratio > 0.75 &&*/)
        {
            balls.push_back(contours[i]);
            ballsBox.push_back(bBox);

            vec2 center;
            center[0] = bBox.x + bBox.width / 2;
            center[1] = bBox.y + bBox.height / 2;
            imagePoints.push_back(vec2(center));

            cv::rectangle(aPreview, bBox, CV_RGB(0, 255, 0), 2);            
        }
    }

    // Keep the three points which are highest, i.e. with lowest Y coordinates
    // That's most usefull to discard noise from features below your cap/head.
    // Typically noise comming from zippers and metal parts on your clothing.
    // With a cap tracker it also successfully discards noise glasses.
    // However it may not work as good with a clip user wearing glasses.
    while (imagePoints.size() > 3) // Until we have no more than three points
    {
        int maxY = 0;
        int index = -1;

        // Search for the point with highest Y coordinate
        for (size_t i = 0; i < imagePoints.size(); i++)
        {
            if (imagePoints[i][1] > maxY)
            {
                maxY = imagePoints[i][1];
                index = i;
            }
        }

        // Discard it
        imagePoints.erase(imagePoints.begin() + index);
    }


}



