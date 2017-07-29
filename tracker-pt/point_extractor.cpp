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
#include <memory>

#include <QDebug>

using namespace types;
using namespace pt_impl;

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
static cv::Vec2d MeanShiftIteration(const cv::Mat &frame_gray, const cv::Vec2d &current_center, double filter_width)
{
    // Most amazingling this function runs faster with doubles than with floats.
    const double s = 1.0 / filter_width;

    double m = 0;
    cv::Vec2d com(0.0, 0.0);
    for (int i = 0; i < frame_gray.rows; i++)
    {
        auto frame_ptr = (uint8_t const * restrict)frame_gray.ptr(i);
        for (int j = 0; j < frame_gray.cols; j++)
        {
            double val = frame_ptr[j];
            val = val * val; // taking the square wights brighter parts of the image stronger.
            {
                double dx = (j - current_center[0])*s;
                double dy = (i - current_center[1])*s;
                double f = std::fmax(0.0, 1.0 - dx*dx - dy*dy);
                val *= f;
            }
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

PointExtractor::PointExtractor()
{
    blobs.reserve(max_blobs);
}

void PointExtractor::extract_points(const cv::Mat& frame, cv::Mat& preview_frame, std::vector<vec2>& points)
{
    using std::sqrt;
    using std::fmax;
    using std::round;
    using std::sort;

    if (frame_gray.rows != frame.rows || frame_gray.cols != frame.cols)
    {
        frame_gray = cv::Mat1b(frame.rows, frame.cols);
        frame_bin = cv::Mat1b(frame.rows, frame.cols);
        frame_blobs = cv::Mat1b(frame.rows, frame.cols);
    }

    // convert to grayscale
    cv::cvtColor(frame, frame_gray, cv::COLOR_BGR2GRAY);

    const double region_size_min = s.min_point_size;
    const double region_size_max = s.max_point_size;

    if (!s.auto_threshold)
    {
        const int thres = s.threshold;
        cv::threshold(frame_gray, frame_bin, thres, 255, cv::THRESH_BINARY);
    }
    else
    {
        int hist_size = 256;
        float ranges_[] = { 0, 256 };
        float const* ranges = (float*) ranges_;

        cv::calcHist(&frame_gray,
                     1,
                     nullptr,
                     cv::noArray(),
                     hist,
                     1,
                     (int const*) &hist_size,
                     &ranges);

        const double cx = frame.cols / 640., cy = frame.rows / 480.;

        const double min_radius = 1.75 * cx;
        const double max_radius = 15 * cy;

        const double radius = fmax(0., (max_radius-min_radius) * s.threshold / 255 + min_radius);
        float const* restrict ptr = reinterpret_cast<float const* restrict>(hist.ptr(0));
        const unsigned area = uround(3 * M_PI * radius*radius);
        const unsigned sz = unsigned(hist.cols * hist.rows);
        unsigned thres = 32;
        for (unsigned i = sz-1, cnt = 0; i > 32; i--)
        {
            cnt += ptr[i];
            if (cnt >= area)
            {
                thres = i;
                break;
            }
        }

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
            if (norm > 0)
            {
                const double radius = sqrt(cnt / M_PI), N = double(norm);
                if (radius > region_size_max || radius < region_size_min)
                    continue;

                blob b(radius, cv::Vec2d(m10 / N, m01 / N), N/sqrt(double(cnt)), rect);
                blobs.push_back(b);

                {
                    static const f offx = 10, offy = 7.5;
                    const f cx = preview_frame.cols / f(frame.cols),
                            cy = preview_frame.rows / f(frame.rows),
                            c_ = (cx+cy)/2;

                    static constexpr unsigned fract_bits = 16;
                    static constexpr double c_fract(1 << fract_bits);

                    cv::Point p(iround(b.pos[0] * cx * c_fract), iround(b.pos[1] * cy * c_fract));

                    cv::circle(preview_frame, p, iround((b.radius + 2) * c_ * c_fract), cv::Scalar(255, 255, 0), 1, cv::LINE_AA, fract_bits);

                    char buf[64];
                    sprintf(buf, "%.2fpx", radius);

                    cv::putText(preview_frame,
                                buf,
                                cv::Point(iround(b.pos[0]*cx+offx), iround(b.pos[1]*cy+offy)),
                                cv::FONT_HERSHEY_PLAIN,
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

    const int W = frame.cols;
    const int H = frame.rows;

    for (idx = 0; idx < std::min(PointModel::N_POINTS, unsigned(blobs.size())); ++idx)
    {
        blob &b = blobs[idx];
        cv::Rect rect = b.rect;

        rect.x -= rect.width / 2;
        rect.y -= rect.height / 2;
        rect.width *= 2;
        rect.height *= 2;
        rect &= cv::Rect(0, 0, W, H);  // crop at frame boundaries

        cv::Mat frame_roi = frame_gray(rect);

        // smaller values mean more changes. 1 makes too many changes while 1.5 makes about .1
        // seems values close to 1.3 reduce noise best with about .15->.2 changes
        static constexpr double radius_c = 1.5;

        const double kernel_radius = b.radius * radius_c;
        cv::Vec2d pos(b.pos[0] - rect.x, b.pos[1] - rect.y); // position relative to ROI.

        for (int iter = 0; iter < 10; ++iter)
        {
            cv::Vec2d com_new = MeanShiftIteration(frame_roi, pos, kernel_radius);
            cv::Vec2d delta = com_new - pos;
            pos = com_new;
            if (delta.dot(delta) < 1e-2)
                break;
        }

        b.pos[0] = pos[0] + rect.x;
        b.pos[1] = pos[1] + rect.y;
    }

    // End of mean shift code. At this point, blob positions are updated which hopefully less noisy less biased values.
    points.reserve(max_blobs);
    points.clear();

    for (const auto& b : blobs)
    {
        // note: H/W is equal to fx/fy

        vec2 p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
}

blob::blob(double radius, const cv::Vec2d& pos, double brightness, cv::Rect& rect) :
    radius(radius), brightness(brightness), pos(pos), rect(rect)
{
    //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1];
}
