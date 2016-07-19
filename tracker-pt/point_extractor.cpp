/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include <QDebug>

#ifdef DEBUG_EXTRACTION
#   include "opentrack-compat/timer.hpp"
#endif

#include <opencv2/highgui.hpp>

#include <cmath>
#include <algorithm>
#include <cinttypes>

PointExtractor::PointExtractor()
{
    blobs.reserve(max_blobs);
}

void PointExtractor::extract_points(cv::Mat& frame, std::vector<PointExtractor::vec2>& points)
{
    const int W = frame.cols;
    const int H = frame.rows;

    if (frame_gray.rows != frame.rows || frame_gray.cols != frame.cols)
    {
        frame_gray = cv::Mat(frame.rows, frame.cols, CV_8U);
        frame_bin = cv::Mat(frame.rows, frame.cols, CV_8U);
        frame_blobs = cv::Mat(frame.rows, frame.cols, CV_8U);
    }

    // convert to grayscale
    cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

    const double region_size_min = s.min_point_size;
    const double region_size_max = s.max_point_size;

    if (!s.auto_threshold)
    {
        const int thres = s.threshold;
        cv::threshold(frame_gray, frame_bin, thres, 255, cv::THRESH_BINARY);
    }
    else
    {
        cv::calcHist(std::vector<cv::Mat> { frame_gray },
                     std::vector<int> { 0 },
                     cv::Mat(),
                     hist,
                     std::vector<int> { 256 },
                     std::vector<float> { 0, 256 },
                     false);
        const int sz = hist.cols * hist.rows;
        int thres = s.threshold;
        int cnt = 0;
        constexpr double min_pixels = 2 * 2 * 3 * pi;
        constexpr double max_pixels = 7 * 7 * 3 * pi;
        constexpr double range_pixels = max_pixels - min_pixels;
        const int pixels_to_include = std::max<int>(0, int(min_pixels + range_pixels * s.threshold / 256));
        auto ptr = reinterpret_cast<const float*>(hist.ptr(0));
        for (int i = sz-1; i > 0; i--)
        {
            cnt += ptr[i];
            if (cnt >= pixels_to_include)
            {
                thres = i;
                break;
            }
        }
        //val *= 240./256.;
        //qDebug() << "val" << val;

        cv::threshold(frame_gray, frame_bin, thres, 255, CV_THRESH_BINARY);
    }

    blobs.clear();
    frame_gray.copyTo(frame_blobs);

    unsigned idx = 0;
    for (int y=0; y < frame_gray.rows; y++)
    {
        if (idx > max_blobs) break;

        const unsigned char* ptr_bin = frame_bin.ptr(y);
        for (int x=0; x < frame_gray.cols; x++)
        {
            if (idx > max_blobs) break;

            if (ptr_bin[x] != 255)
                continue;
            idx = blobs.size();
            cv::Rect rect;
            cv::floodFill(frame_blobs,
                          cv::Point(x,y),
                          cv::Scalar(idx),
                          &rect,
                          cv::Scalar(0),
                          cv::Scalar(0),
                          4);
            long m00 = 0;
            long m10 = 0;
            long m01 = 0;
            long cnt = 0;
            for (int i=rect.y; i < (rect.y+rect.height); i++)
            {
                const unsigned char* ptr_blobs = frame_blobs.ptr(i);
                const unsigned char* ptr_gray = frame_gray.ptr(i);
                unsigned char* ptr_bin = frame_bin.ptr(i);
                for (int j=rect.x; j < (rect.x+rect.width); j++)
                {
                    if (ptr_blobs[j] != idx) continue;
                    ptr_bin[j] = 0;
                    const long val = ptr_gray[j];
                    m00 += val;
                    m01 += i * val;
                    m10 += j * val;
                    cnt++;
                }
            }
            if (m00 > 0)
            {
                const double radius = std::sqrt(cnt / pi);
                if (radius > region_size_max || radius < region_size_min)
                    continue;
                const double norm = double(m00);
                blob b(radius, cv::Vec2d(m10 / norm, m01 / norm));
                blobs.push_back(b);
                {
                    char buf[64];
                    sprintf(buf, "%.2fpx", radius);
                    cv::putText(frame,
                                buf,
                                cv::Point((int)std::round(b.pos[0]+30), (int)std::round(b.pos[1]+20)),
                                cv::FONT_HERSHEY_DUPLEX,
                                1,
                                cv::Scalar(0, 0, 255),
                                1);
                }
            }
        }
    }

    std::sort(blobs.begin(), blobs.end(), [](const blob& b1, const blob& b2) -> bool { return b2.radius < b1.radius; });

    points.reserve(max_blobs);
    points.clear();

    for (auto& b : blobs)
    {
        vec2 p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
}
