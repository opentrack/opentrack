/* 
 * Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point-extractor.h"
#include "preview.h"
#include "tracker-easy.h"

#include "cv/numeric.hpp"
#include "compat/math.hpp"

#include <cmath>
#include <algorithm>
#include <cinttypes>
#include <memory>
#include <opencv2/highgui/highgui.hpp>
// Needed for mean shift
#include <opencv2/video/tracking.hpp>

#include <QDebug>


using namespace numeric_types;

namespace EasyTracker
{

    PointExtractor::PointExtractor() : iSettings(KModuleName)
    {
        UpdateSettings();
    }

    ///
    void PointExtractor::UpdateSettings()
    {
        iMinPointSize = iSettings.iMinBlobSize;
        iMaxPointSize = iSettings.iMaxBlobSize;
    }

    ///
    void PointExtractor::ExtractPoints(const cv::Mat& aFrame, cv::Mat* aPreview, int aNeededPointCount, std::vector<cv::Point>& aPoints)
    {
        //TODO: Assert if channel size is neither one nor two
        // Make sure our frame channel is 8 bit
        size_t channelSize = aFrame.elemSize1();
        if (channelSize == 2)
        {
            // We have a 16 bits single channel. Typically coming from Kinect V2 IR sensor
            // Resample to 8-bits
            double min = std::numeric_limits<uint16_t>::min();
            double max = std::numeric_limits<uint16_t>::max();
            //cv::minMaxLoc(raw, &min, &max); // Should we use 16bit min and max instead?
            // For scalling to have more precission in the range we are interrested in
            min = max - 255;
            // See: https://stackoverflow.com/questions/14539498/change-type-of-mat-object-from-cv-32f-to-cv-8u/14539652
            aFrame.convertTo(iFrameChannelSizeOne, CV_8U, 255.0 / (max - min), -255.0*min / (max - min));
        }
        else
        {
            iFrameChannelSizeOne = aFrame;
        }


        // Make sure our frame has a single channel
        // Make an extra copy if needed
        const int channelCount = iFrameChannelSizeOne.channels();
        if (channelCount == 3)
        {
            // Convert to grayscale
            // TODO: What's our input format, BRG or RGB?
            // That won't make our point extraction work but at least it won't crash
            cv::cvtColor(iFrameChannelSizeOne, iFrameGray, cv::COLOR_BGR2GRAY);
            // TODO: Instead convert to HSV and use a key color together with cv::inRange to sport the color we want.
            // Key color should be defined in settings.
        }
        else if (channelCount == 1)
        {
            // No further convertion needed
            iFrameGray = iFrameChannelSizeOne;
        }
        else
        {
            eval_once(qDebug() << "tracker/easy: camera frame depth not supported" << aFrame.channels());
            return;
        }

//#define DEBUG
#ifdef DEBUG        
        cv::imshow("iFrameGray", iFrameGray);
        cv::erode(iFrameGray, iFrameGray, cv::Mat(), cv::Point(-1, -1), 1);
        cv::imshow("Eroded", iFrameGray);
        cv::dilate(iFrameGray, iFrameGray, cv::Mat(), cv::Point(-1, -1), 2);
        cv::imshow("Dilated", iFrameGray);        
#endif 


        // Contours detection
        iContours.clear();
        cv::findContours(iFrameGray, iContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    
        // Workout which countours are valid points
        for (size_t i = 0; i < iContours.size(); i++)
        {
            if (aPreview)
            {
                cv::drawContours(*aPreview, iContours, (int)i, CV_RGB(255, 0, 0), 2);
            }
        

            cv::Rect bBox;
            bBox = cv::boundingRect(iContours[i]);
       
            // Make sure bounding box matches our criteria
            if (bBox.width >= iMinPointSize
                && bBox.height >= iMinPointSize
                && bBox.width <= iMaxPointSize
                && bBox.height <= iMaxPointSize)
            {
                // Do a mean shift or cam shift, it's not bringing much though
                //cv::CamShift(iFrameGray, bBox, cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 10, 1));
                //                
                cv::Point center;
                center.x = bBox.x + bBox.width / 2;
                center.y = bBox.y + bBox.height / 2;
                aPoints.push_back(center);

                if (aPreview)
                {
                    cv::rectangle(*aPreview, bBox, CV_RGB(0, 255, 0), 2);
                }
            }
        }

        // Keep only the three points which are highest, i.e. with lowest Y coordinates
        // That's most usefull to discard noise from features below your cap/head.
        // Typically noise comming from zippers and metal parts on your clothing.
        // With a cap tracker it also successfully discards noise from glasses.
        // However it may not work as good with a clip user wearing glasses.
        while (aPoints.size() > aNeededPointCount) // Until we have no more than three points
        {
            int maxY = 0;
            size_t index = std::numeric_limits<size_t>::max();

            // Search for the point with highest Y coordinate
            for (size_t i = 0; i < aPoints.size(); i++)
            {
                if (aPoints[i].y > maxY)
                {
                    maxY = aPoints[i].y;
                    index = i;
                }
            }
            
            if (index < aPoints.size()) // Defensive
            {
                // Discard it
                aPoints.erase(aPoints.begin() + index);
            }
            
        }
    }

}

