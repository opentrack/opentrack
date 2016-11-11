/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include "compat/util.hpp"

#include <opencv2/videoio.hpp>

#include <cmath>
#include <algorithm>
#include <cinttypes>

#include <QDebug>

using namespace types;

PointExtractor::PointExtractor()
{
    blobs.reserve(max_blobs);
}

void PointExtractor::extract_points(cv::Mat& frame, std::vector<vec2>& points)
{
    using std::sqrt;
    using std::max;
    using std::round;
    using std::sort;

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

        static constexpr double min_radius = 2.5;
        static constexpr double max_radius = 15;

        const double radius = max(0., (max_radius-min_radius) * s.threshold / 255 + min_radius);
        const float* ptr = reinterpret_cast<const float*>(hist.ptr(0));
        const unsigned area = unsigned(round(3 * M_PI * radius*radius));
        const unsigned sz = unsigned(hist.cols * hist.rows);
        unsigned thres = 1;
        for (unsigned i = sz-1, cnt = 0; i > 1; i--)
        {
            cnt += ptr[i];
            if (cnt >= area)
            {
                thres = i;
                break;
            }
        }
        //val *= 240./256.;
        //qDebug() << "thres" << thres;

        cv::threshold(frame_gray, frame_bin, thres, 255, CV_THRESH_BINARY);
    }

    blobs.clear();
    frame_bin.copyTo(frame_blobs);

    unsigned idx = 0;
    for (int y=0; y < frame_blobs.rows; y++)
    {
        const unsigned char* ptr_bin = frame_blobs.ptr(y);
        for (int x=0; x < frame_blobs.cols; x++)
        {
            if (ptr_bin[x] != 255)
                continue;
            idx = blobs.size() + 1;

            cv::Rect rect;
            cv::floodFill(frame_blobs,
                          cv::Point(x,y),
                          cv::Scalar(idx),
                          &rect,
                          cv::Scalar(0),
                          cv::Scalar(0),
                          8);

            // these are doubles since m10 and m01 could overflow theoretically
            // log2(255^2 * 640^2 * pi) > 36
            double m10 = 0;
            double m01 = 0;
            // norm can't overflow since there's no 640^2 component
            int norm = 0;
            int cnt = 0;

            for (int i=rect.y; i < (rect.y+rect.height); i++)
            {
                unsigned char* ptr_blobs = frame_blobs.ptr(i);
                const unsigned char* ptr_gray = frame_gray.ptr(i);
                for (int j=rect.x; j < (rect.x+rect.width); j++)
                {
                    if (ptr_blobs[j] != idx)
                        continue;

                    ptr_blobs[j] = 0;

                    // square as a weight gives better results
                    const int val(int(ptr_gray[j]) * int(ptr_gray[j]));

                    norm += val;
                    m01 += i * val;
                    m10 += j * val;
                    cnt++;
                }
            }
            if (norm > 1e0)
            {
                const double radius = sqrt(cnt / M_PI);
                if (radius > region_size_max || radius < region_size_min)
                    continue;
                blob b(radius, cv::Vec2d(m10 / norm, m01 / norm), norm/sqrt(double(cnt)));
                blobs.push_back(b);
                {
                    char buf[64];
                    sprintf(buf, "%.2fpx", radius);
                    cv::putText(frame,
                                buf,
                                cv::Point((int)round(b.pos[0]+30), (int)round(b.pos[1]+20)),
                                cv::FONT_HERSHEY_DUPLEX,
                                1,
                                cv::Scalar(0, 0, 255),
                                1);
                }

                if (idx >= max_blobs) goto end;
            }
        }
    }
end:

    sort(blobs.begin(), blobs.end(), [](const blob& b1, const blob& b2) -> bool { return b2.brightness < b1.brightness; });

    points.reserve(max_blobs);
    points.clear();

    for (auto& b : blobs)
    {
        vec2 p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
}
