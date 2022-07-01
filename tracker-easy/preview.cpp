/* 
 * Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "preview.h"

#include "compat/math.hpp"
#include "compat/macros.h"

#include <opencv2/imgproc.hpp>
#include <QDebug>


namespace EasyTracker
{


    Preview& Preview::operator=(const cv::Mat& aFrame)
    {
        //TODO: Assert if channel size is neither one nor two

        // Make sure our frame channel is 8 bit
        size_t channelSize = aFrame.elemSize1();
        if (channelSize == 2)
        {
            // First resample to 8-bits            
            double min = std::numeric_limits<uint16_t>::min();
            double max = std::numeric_limits<uint16_t>::max();
            //cv::minMaxLoc(raw, &min, &max); // Should we use 16bit min and max instead?
            // For scalling to have more precission in the range we are interrested in
            //min = max - 255;
            // See: https://stackoverflow.com/questions/14539498/change-type-of-mat-object-from-cv-32f-to-cv-8u/14539652
            aFrame.convertTo(iFrameChannelSizeOne, CV_8U, 255.0 / (max - min), -255.0*min / (max - min));
        }
        else
        {
            iFrameChannelSizeOne = aFrame;
        }

        // Make sure our frame is RGB
        // Make an extra copy if needed
        int channelCount = iFrameChannelSizeOne.channels();
        if (channelCount == 1)
        {
            // Convert to RGB
            cv::cvtColor(iFrameChannelSizeOne, iFrameRgb, cv::COLOR_GRAY2BGR);
        }
        else if (channelCount == 3)
        {
            iFrameRgb = iFrameChannelSizeOne;
        }
        else
        {
            eval_once(qDebug() << "tracker/easy: camera frame depth not supported" << aFrame.channels());
            return *this;
        }


        return *this;
    }

    Preview::Preview(int w, int h)
    {
        ensure_size(iFrameWidget, w, h, CV_8UC4);
        ensure_size(iFrameResized, w, h, CV_8UC3);

        iFrameResized.setTo(cv::Scalar(0, 0, 0));
    }

    QImage Preview::get_bitmap()
    {
        size_t stride = iFrameWidget.step.p[0];

        if (stride < 64 || stride < iFrameWidget.cols * 4)
        {
            eval_once(qDebug() << "bad stride" << stride
                << "for bitmap size" << iFrameResized.cols << iFrameResized.rows);
            return QImage();
        }

        // Resize if needed
        const bool need_resize = iFrameRgb.cols != iFrameWidget.cols || iFrameRgb.rows != iFrameWidget.rows;
        if (need_resize)
        {
            cv::resize(iFrameRgb, iFrameResized, cv::Size(iFrameWidget.cols, iFrameWidget.rows), 0, 0, cv::INTER_NEAREST);
        }
        else
        {
            iFrameResized = iFrameRgb;
        }

        cv::cvtColor(iFrameResized, iFrameWidget, cv::COLOR_BGR2BGRA);

        return QImage((const unsigned char*)iFrameWidget.data,
            iFrameWidget.cols, iFrameWidget.rows,
            (int)stride,
            QImage::Format_ARGB32);
    }

    void Preview::DrawInfo(const std::string& aString)
    {
        cv::putText(iFrameRgb, aString, cv::Point(4,iFrameRgb.size().height-4), cv::FONT_HERSHEY_PLAIN, 2, cv::Scalar(0, 255, 0),2);
    }

    void Preview::DrawCross(const cv::Point& aPoint, const cv::Scalar& aColor)
    {
        constexpr int len = 9;        
        cv::line(iFrameRgb,
            cv::Point(aPoint.x - len, aPoint.y),
            cv::Point(aPoint.x + len, aPoint.y),
            aColor, 2);
        cv::line(iFrameRgb,
            cv::Point(aPoint.x, aPoint.y - len),
            cv::Point(aPoint.x, aPoint.y + len),
            aColor, 2);
    }

    void Preview::ensure_size(cv::Mat& frame, int w, int h, int type)
    {
        if (frame.cols != w || frame.rows != h || frame.type() != type)
            frame = cv::Mat(h, w, type);
    }

}
