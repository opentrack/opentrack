/*
 * Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */


#pragma once

#include <opencv2/core.hpp>
#include <QImage>

namespace EasyTracker
{

    struct Preview
    {
        Preview(int w, int h);

        Preview& operator=(const cv::Mat& frame);
        QImage get_bitmap();
        void DrawCross(const cv::Point& aPoint, const cv::Scalar& aColor);
        void DrawInfo(const std::string& aString);

        operator cv::Mat&() { return iFrameResized; }
        operator cv::Mat const&() const { return iFrameResized; }
       
    private:
        static void ensure_size(cv::Mat& frame, int w, int h, int type);

    public:
        // Frame with an 8 bits channel size
        cv::Mat iFrameChannelSizeOne;
        // Frame with three channels for RGB
        cv::Mat iFrameRgb;
        // Frame rezised to widget dimension
        cv::Mat iFrameResized;
        // Frame ready to be packed in a QImage for use in our widget
        cv::Mat iFrameWidget;
        
    };

}

