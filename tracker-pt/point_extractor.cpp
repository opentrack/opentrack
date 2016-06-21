/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
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

#include <cmath>

PointExtractor::PointExtractor()
{
    blobs.reserve(max_blobs);
    points.reserve(max_blobs);
}

const std::vector<cv::Vec2f>& PointExtractor::extract_points(cv::Mat& frame)
{
    const int W = frame.cols;
    const int H = frame.rows;

    if (frame_gray.rows != frame.rows || frame_gray.cols != frame.cols)
    {
        frame_gray = cv::Mat(frame.rows, frame.cols, CV_8U);
        frame_bin = cv::Mat(frame.rows, frame.cols, CV_8U);;
    }

    // convert to grayscale
    cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

    const double region_size_min = s.min_point_size;
    const double region_size_max = s.max_point_size;

    const int thres = s.threshold;

    contours.clear();

    if (!s.auto_threshold)
    {
        cv::threshold(frame_gray, frame_bin, thres, 255, cv::THRESH_BINARY);
        cv::findContours(frame_bin, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    }
    else
    {
        cv::calcHist(std::vector<cv::Mat> { frame_gray },
                     std::vector<int> { 0 },
                     cv::Mat(),
                     hist,
                     std::vector<int> { 256/hist_c },
                     std::vector<float> { 0, 256/hist_c },
                     false);
        const int sz = hist.cols * hist.rows;
        int val = 0;
        int cnt = 0;
        constexpr int min_pixels = int(10 * 10 * 3 * pi);
        const int pixels_to_include = std::max<int>(0, min_pixels * s.threshold / 255);
        auto ptr = reinterpret_cast<const float*>(hist.ptr(0));
        for (int i = sz-1; i >= 0; i--)
        {
            cnt += ptr[i];
            if (cnt >= pixels_to_include)
            {
                val = i;
                break;
            }
        }
        val *= hist_c;
        val *= 240./256.;
        //qDebug() << "val" << val;

        cv::threshold(frame_gray, frame_bin, val, 255, CV_THRESH_BINARY);
        cv::findContours(frame_bin, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    }

    blobs.clear();

    for (auto& c : contours)
    {
        using std::fabs;

        const auto m = cv::moments(cv::Mat(c));

        if (fabs(m.m00) < 1e-3)
            continue;

        const cv::Vec2d pos(m.m10 / m.m00, m.m01 / m.m00);

        double radius;
// following based on OpenCV SimpleBlobDetector
        {
            std::vector<double> dists;
            for (auto& k : c)
            {
                dists.push_back(cv::norm(pos - cv::Vec2d(k.x, k.y)));
            }
            std::sort(dists.begin(), dists.end());
            radius = (dists[(dists.size() - 1)/2] + dists[dists.size()/2])/2;
        }

        if (radius < region_size_min || radius > region_size_max)
            continue;

        double confid = 1;
        {
            double denominator = std::sqrt(std::pow(2 * m.mu11, 2) + std::pow(m.mu20 - m.mu02, 2));
            const double eps = 1e-2;
            if (denominator > eps)
            {
                double cosmin = (m.mu20 - m.mu02) / denominator;
                double sinmin = 2 * m.mu11 / denominator;
                double cosmax = -cosmin;
                double sinmax = -sinmin;

                double imin = 0.5 * (m.mu20 + m.mu02) - 0.5 * (m.mu20 - m.mu02) * cosmin - m.mu11 * sinmin;
                double imax = 0.5 * (m.mu20 + m.mu02) - 0.5 * (m.mu20 - m.mu02) * cosmax - m.mu11 * sinmax;
                confid = imin / imax;
            }
        }
// end SimpleBlobDetector

        {
            char buf[64];
            sprintf(buf, "%.2fpx %.2fc", radius, confid);
            cv::putText(frame, buf, cv::Point(pos[0]+30, pos[1]+20), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 255), 1);
        }

        blobs.push_back(blob(radius, pos, confid));

        if (blobs.size() == max_blobs)
            break;
    }

    using b = const blob;
    std::sort(blobs.begin(), blobs.end(), [](b& b1, b& b2) {return b1.confid > b2.confid;});

    QMutexLocker l(&mtx);
    points.clear();

    for (auto& b : blobs)
    {
        cv::Vec2f p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
    return points;
}
