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

//#define DEBUG_SUM_OF_SQUARES
#ifdef DEBUG_SUM_OF_SQUARES
#   define SUM_OF_SQUARES_WINNAME "sum-of-squares-debug"
#   include <opencv2/highgui.hpp>
#endif

PointExtractor::PointExtractor()
{
#ifdef DEBUG_SUM_OF_SQUARES
    cv::namedWindow(SUM_OF_SQUARES_WINNAME);
#endif
    blobs.reserve(max_blobs);
    points.reserve(max_blobs);
}

PointExtractor::~PointExtractor()
{
#ifdef DEBUG_SUM_OF_SQUARES
    cv::destroyWindow(SUM_OF_SQUARES_WINNAME);
#endif
}

void PointExtractor::gray_square_diff(const cv::Mat &frame, cv::Mat &frame_gray)
{
    const unsigned nchans = frame.channels();
    const int rows = frame.rows;
    const int cols = frame.cols;
    cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

    if (nchans == 1 || !s.auto_threshold)
        return;

    cv::split(frame, gray_split_channels);

    if (nchans > gray_absdiff_channels.size())
        gray_absdiff_channels.resize(nchans);

    for (unsigned i = 0; i < nchans; i++)
        cv::absdiff(frame_gray, gray_split_channels[i], gray_absdiff_channels[i]);

    if (frame_gray_tmp.rows != rows || frame_gray_tmp.cols != cols)
        frame_gray_tmp = cv::Mat(rows, cols, CV_32FC1);

    frame_gray.convertTo(frame_gray_tmp, CV_32FC1);

    constexpr float scale = .9;

    if (float_absdiff_channel.cols != cols || float_absdiff_channel.rows != rows)
        float_absdiff_channel = cv::Mat(rows, cols, CV_32FC1);

    for (unsigned i = 0; i < nchans; i++)
    {
        gray_absdiff_channels[i].convertTo(float_absdiff_channel, CV_32FC1);

        frame_gray_tmp -= float_absdiff_channel.mul(float_absdiff_channel, scale);
    }

    frame_gray_tmp.convertTo(frame_gray, CV_8UC1);

#ifdef DEBUG_SUM_OF_SQUARES
    cv::imshow(SUM_OF_SQUARES_WINNAME, frame_gray);
    cv::waitKey(1);
#endif
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
    gray_square_diff(frame, frame_gray);

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
        constexpr int min_pixels = 20;
        const auto pixels_to_include = std::max<int>(0, min_pixels * s.threshold/100.);
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
        const auto m = cv::moments(cv::Mat(c));
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
