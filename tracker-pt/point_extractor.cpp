/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2015-2016 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include "compat/util.hpp"
#include "point_tracker.h"
#include <QDebug>

#include <opencv2/videoio.hpp>

#include <cmath>
#include <algorithm>
#include <cinttypes>

#include <QDebug>

using namespace types;

/*
http://en.wikipedia.org/wiki/Mean-shift
In this application the idea, is to eliminate any bias of the point estimate 
which is introduced by the rather arbitrary thresholded area. One must recognize
that the thresholded area can only move in one pixel increments since it is
binary. Thus, its center of mass might make "jumps" as pixels are added/removed
from the thresholded area.
With mean-shift, a moving "window" or kernel is multiplied with the gray-scale
image, and the COM is calculated of the result. This is iterated where the
kernel center is set the previously computed COM. Thus, peaks in the image intensity
distribution "pull" the kernel towards themselves. Eventually it stops moving, i.e.
then the computed COM coincides with the kernel center. We hope that the 
corresponding location is a good candidate for the extracted point.
The idea similar to the window scaling suggested in  Berglund et al. "Fast, bias-free 
algorithm for tracking single particles with variable size and shape." (2008).
*/
static cv::Vec2d MeanShiftIteration(const cv::Mat& frame_gray, const cv::Vec2d& current_center, double filter_width)
{
    // Most amazingling this function runs faster with doubles than with floats.
    const double s = 1.0 / filter_width;

    double m = 0;
    cv::Vec2d com(0.0, 0.0);
    const int h = frame_gray.rows, w = frame_gray.cols;
    for (int i = 0; i < h; i++)
    {
        auto frame_ptr = (const unsigned char* OTR_RESTRICT)frame_gray.ptr(i);
        for (int j = 0; j < w; j++)
        {
            double val = frame_ptr[j];
            double dx = (j - current_center[0])*s;
            double dy = (i - current_center[1])*s;
            double f = std::max(0.0, 1.0 - dx*dx - dy*dy);
            val *= f;
            m += val;
            com[0] += j * val;
            com[1] += i * val;
        }
    }
    if (m > 0.1)
    {
        com *= 1.0 / m;
        return com;
    }
    else
        return current_center;
}
// End of mean shift code. At this point, blob positions are updated which hopefully less noisy less biased values.

PointExtractor::PointExtractor()
{
    blobs.reserve(max_blobs);
}

void PointExtractor::extract_points(const cv::Mat& frame, cv::Mat& preview_frame, std::vector<vec2>& points)
{
    using std::sqrt;
    using std::max;
    using std::round;
    using std::sort;

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
        const float* ptr = reinterpret_cast<const float* OTR_RESTRICT>(hist.ptr(0));
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
    const int h = frame_blobs.rows, w = frame_blobs.cols;
    for (int y=0; y < h; y++)
    {
        const unsigned char* OTR_RESTRICT ptr_bin = frame_blobs.ptr(y);
        for (int x=0; x < w; x++)
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
                          cv::Scalar(0));

            const double radius = sqrt(rect.width * rect.height / M_PI);
            if (radius > region_size_max || radius < region_size_min)
                continue;

            blobs.push_back(blob { radius, cv::Vec2d(rect.x/* + rect.width/2.*/, rect.y/* + rect.height/2.*/), rect });

            if (idx >= max_blobs) goto end;
        }
    }
end:

    sort(blobs.begin(), blobs.end(), [](const blob& b1, const blob& b2) { return b2.radius < b1.radius; });

    const int W = frame.cols;
    const int H = frame.rows;

    for (idx = 0; idx < std::min(PointModel::N_POINTS, unsigned(blobs.size())); ++idx)
    {
        blob &b = blobs[idx];
        cv::Rect rect = b.rect;

        cv::Mat frame_roi = frame_gray(rect);

        static constexpr double radius_c = 2;

        const double kernel_radius = b.radius * radius_c;
        cv::Vec2d pos(b.pos[0] - rect.x, b.pos[1] - rect.y); // position relative to ROI.

        int iter; (void)iter;
        for (iter = 0; iter < 10; ++iter)
        {
            cv::Vec2d com_new = MeanShiftIteration(frame_roi, com_new, kernel_radius);
            cv::Vec2d delta = com_new - pos;
            pos = com_new;
            if (delta.dot(delta) < 1e-3)
                break;
        }

        b.pos = cv::Vec2d(pos[0] + rect.x, pos[1] + rect.y);

        {
            static const f offx = 10, offy = 7.5;
            const f cx = preview_frame.cols / f(frame.cols),
                    cy = preview_frame.rows / f(frame.rows),
                    c  = .5*(cx+cy);
            cv::Point p(iround(b.pos[0] * cx), iround(b.pos[1] * cy));

            static const cv::Scalar color(0, 255, 0);

            {
                const int len = iround(b.radius * c * 1.1) + 1;
                cv::line(preview_frame,
                         cv::Point(p.x - len, p.y),
                         cv::Point(p.x + len, p.y),
                         color);
                cv::line(preview_frame,
                         cv::Point(p.x, p.y - len),
                         cv::Point(p.x, p.y + len),
                         color);
            }

            char buf[32];
            sprintf(buf, "%.2fpx", b.radius);

            cv::putText(preview_frame,
                        buf,
                        cv::Point(iround(b.pos[0]*cx+offx), iround(b.pos[1]*cy+offy)),
                        cv::FONT_HERSHEY_PLAIN,
                        1,
                        cv::Scalar(0, 0, 255),
                        1);
        }
    }

    points.reserve(max_blobs);
    points.clear();

    for (const auto& b : blobs)
    {
        // note: H/W is equal to fx/fy

        vec2 p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
}

