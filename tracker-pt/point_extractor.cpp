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
#include <opencv2/imgproc.hpp>

#include <cmath>
#include <algorithm>
#include <cinttypes>
#include <vector>
#include <array>

using std::sqrt;
using std::fmax;
using std::min;

using namespace pt_extractor_impl;

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
static cv::Vec2d MeanShiftIteration(const cv::Mat &frame_gray, const vec2 &current_center, f filter_width)
{
    // Most amazingling this function runs faster with doubles than with floats.
    const f s = 1 / filter_width;

    f m = 0;
    vec2 com(0, 0);
    for (int i = 0; i < frame_gray.rows; i++)
    {
        const auto frame_ptr = (const std::uint8_t*)frame_gray.ptr(i);
        for (int j = 0; j < frame_gray.cols; j++)
        {
            f val = frame_ptr[j];
            val = val * val; // taking the square wights brighter parts of the image stronger.
            {
                f dx = (j - current_center[0])*s;
                f dy = (i - current_center[1])*s;
                f f = fmax(0.0, 1 - dx*dx - dy*dy);
                val *= f;
            }
            m += val;
            com[0] += j * val;
            com[1] += i * val;
        }
    }
    if (m > f(.1))
    {
        com *= 1 / m;
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
    if (frame_gray.rows != frame.rows || frame_gray.cols != frame.cols)
    {
        frame_gray = cv::Mat(frame.rows, frame.cols, CV_8U);
        frame_bin = cv::Mat(frame.rows, frame.cols, CV_8U);
        //frame_blobs = cv::Mat(frame.rows, frame.cols, CV_8U);
    }

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
        static const std::vector<int> used_channels { 0 };
        static const std::vector<int> hist_size { 256 };
        static const std::vector<float> hist_ranges { 0, 256 };

        cv::calcHist(std::vector<cv::Mat1b> { frame_gray },
                     used_channels,
                     cv::noArray(),
                     hist,
                     hist_size,
                     hist_ranges,
                     false);

        static constexpr double min_radius = 2.5;
        static constexpr double max_radius = 15;

        const float* restrict ptr = reinterpret_cast<const float*>(hist.data);
        const double radius = fmax(0., (max_radius-min_radius) * s.threshold / 255 + min_radius);
        const unsigned area = uround(3 * M_PI * radius * radius);
        unsigned thres = 255;
        unsigned accum = 0;

        for (unsigned k = 255; k != 0; k--)
        {
            accum += ptr[k];
            if (accum >= area)
            {
                thres = k;
                break;
            }
        }

        cv::threshold(frame_gray, frame_bin, thres, 255, cv::THRESH_BINARY);
    }

    blobs.clear();

    // -----
    // start code borrowed from OpenCV's modules/features2d/src/blobdetector.cpp
    // -----

    contours.clear();

    cv::findContours(frame_bin, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE);
    const unsigned cnt = std::min<unsigned>(max_blobs, contours.size());

    for (unsigned k = 0; k < cnt; k++)
    {
        if (contours[k].size() == 0)
            continue;

        cv::Moments moments = cv::moments(contours[k]);

        const double area = moments.m00;
        const double radius = std::sqrt(area) / std::sqrt(M_PI);

        if (radius < std::fmax(2.5, region_size_min) || (radius > region_size_max))
            continue;

        cv::Rect rect = cv::boundingRect(contours[k]) & cv::Rect(0, 0, frame.cols, frame.rows);

        rect &= cv::Rect(0, 0, frame.cols, frame.rows); // crop at frame boundaries

        if (rect.width == 0 || rect.height == 0)
            continue;

        const vec2 center(moments.m10 / moments.m00, moments.m01 / moments.m00);

        if (!cv::Point2d(center).inside(cv::Rect2d(rect)))
            continue;

        const double value = radius;

        blob b(radius, center, value, rect);

        blobs.push_back(b);

        static const f offx = 10, offy = 7.5;
        const f cx = preview_frame.cols / f(frame.cols),
                cy = preview_frame.rows / f(frame.rows),
                c_ = (cx+cy)/2;

        static constexpr unsigned fract_bits = 16;
        static constexpr double c_fract(1 << fract_bits);

        cv::Point p(iround(b.pos[0] * cx * c_fract), iround(b.pos[1] * cy * c_fract));

        cv::circle(preview_frame, p, iround((b.radius + 2) * c_ * c_fract), cv::Scalar(255, 255, 0), 1, cv::LINE_AA, fract_bits);
        cv::circle(preview_frame, p, 1, cv::Scalar(255, 255, 64), -1, cv::LINE_4);

        char buf[64];
        sprintf(buf, "%.1fpx", int(b.radius*10+.5)/10.);

        cv::putText(preview_frame,
                    buf,
                    cv::Point(iround(b.pos[0]*cx+offx), iround(b.pos[1]*cy+offy)),
                    cv::FONT_HERSHEY_PLAIN,
                    1,
                    cv::Scalar(0, 0, 255),
                    1);
    }
    // -----
    // end of code borrowed from OpenCV's modules/features2d/src/blobdetector.cpp
    // -----

    std::sort(blobs.begin(), blobs.end(), [](const blob& b1, const blob& b2) { return b2.value < b1.value; });

    const int W = frame.cols;
    const int H = frame.rows;

#if defined DEBUG_MEANSHIFT
    double meanshift_total = 0;
#endif

    for (unsigned k = 0; k < min(PointModel::N_POINTS, unsigned(blobs.size())); ++k)
    {
        blob &b = blobs[k];
        const cv::Rect rect = b.rect;

        cv::Mat frame_roi = frame_gray(rect);

        static constexpr f radius_c = 1.75;

        const f kernel_radius = b.radius * radius_c;
        cv::Vec2d pos(b.pos[0] - rect.x, b.pos[1] - rect.y); // position relative to ROI.
#if defined DEBUG_MEANSHIFT
        cv::Vec2d pos_(pos);
#endif

        for (int iter = 0; iter < 10; ++iter)
        {
            cv::Vec2d com_new = MeanShiftIteration(frame_roi, pos, kernel_radius);
            cv::Vec2d delta = com_new - pos;
            pos = com_new;
            if (delta.dot(delta) < 1e-3)
                break;
        }

#if defined DEBUG_MEANSHIFT
        meanshift_total += std::sqrt((pos_ - pos).dot(pos_ - pos));
#endif

        b.pos[0] = pos[0] + rect.x;
        b.pos[1] = pos[1] + rect.y;

        if (!cv::Point2d(b.pos[0], b.pos[1]).inside(b.rect))
            continue;
    }

#if defined DEBUG_MEANSHIFT
    qDebug() << "meanshift adjust total" << meanshift_total;
#endif

    // End of mean shift code. At this point, blob positions are updated with hopefully less noisy, less biased values.
    points.reserve(max_blobs);
    points.clear();

    for (const auto& b : blobs)
    {
        // note: H/W is equal to fx/fy

        vec2 p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
}

blob::blob(double radius, const cv::Vec2d& pos, double brightness, const cv::Rect& rect) :
    radius(radius), value(brightness), pos(pos), rect(rect)
{
    //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1];
}
