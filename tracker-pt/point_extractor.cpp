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

PointExtractor::PointExtractor(){
	//if (!AllocConsole()){}
	//else SetConsoleTitle("debug");
	//freopen("CON", "w", stdout);
	//freopen("CON", "w", stderr);
}
// ----------------------------------------------------------------------------
std::vector<cv::Vec2f> PointExtractor::extract_points(cv::Mat& frame)
{
    const int W = frame.cols;
    const int H = frame.rows;

    // convert to grayscale
    cv::Mat frame_gray;
    cv::cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);

    const double region_size_min = s.min_point_size;
    const double region_size_max = s.max_point_size;
    
    struct blob
    {
        double radius;
        cv::Vec2d pos;
        double confid;
        bool taken;
        double area;
        blob(double radius, const cv::Vec2d& pos, double confid, double area) : radius(radius), pos(pos), confid(confid), taken(false), area(area)
        {
            //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1] << "confid" << confid;
        }
        bool inside(const blob& other)
        {
            cv::Vec2d tmp = pos - other.pos;
            return sqrt(tmp.dot(tmp)) < radius;
        }
    };
    
    // mask for everything that passes the threshold (or: the upper threshold of the hysteresis)
    cv::Mat frame_bin = cv::Mat::zeros(H, W, CV_8U);
    
    std::vector<blob> blobs;
    std::vector<std::vector<cv::Point>> contours;

    const int thres = s.threshold;
    if (!s.auto_threshold)
    {
        cv::Mat frame_bin_;
        cv::threshold(frame_gray, frame_bin_, thres, 255, cv::THRESH_BINARY);
        frame_bin.setTo(170, frame_bin_);
        cv::findContours(frame_bin_, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    }
    else
    {
        cv::Mat hist;
        constexpr int hist_c = 6;
        cv::calcHist(std::vector<cv::Mat> { frame_gray },
                     std::vector<int> { 0 },
                     cv::Mat(),
                     hist,
                     std::vector<int> { 256/hist_c },
                     std::vector<float> { 0, 256/hist_c },
                     false);
        const int sz = hist.rows*hist.cols;
        int val = 0;
        int cnt = 0;
        constexpr int min_pixels = 250;
        const auto pixels_to_include = std::max<int>(0, min_pixels * s.threshold/100.);
        for (int i = sz-1; i >= 0; i--)
        {
            cnt += hist.at<float>(i);
            if (cnt >= pixels_to_include)
            {
                val = i;
                break;
            }
        }
        val *= hist_c;
        val *= 240./256.;
        //qDebug() << "val" << val;

        cv::Mat frame_bin_;
        cv::threshold(frame_gray, frame_bin_, val, 255, CV_THRESH_BINARY);
        frame_bin.setTo(170, frame_bin_);
        cv::findContours(frame_bin_, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
    }

    int cnt = 0;

    for (auto& c : contours)
    {
        if (cnt++ > 30)
            break;

        const auto m = cv::moments(cv::Mat(c));
        const double area = m.m00;
        if (area == 0.)
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

        blobs.push_back(blob(radius, pos, confid, area));
    }
    
    // clear old points
	points.clear();

    using b = const blob;
    std::sort(blobs.begin(), blobs.end(), [](b& b1, b& b2) {return b1.confid > b2.confid;});
    
    for (auto& b : blobs)
    {
        cv::Vec2f p((b.pos[0] - W/2)/W, -(b.pos[1] - H/2)/W);
        points.push_back(p);
    }
    
    // draw output image
    std::vector<cv::Mat> channels_;
    cv::split(frame, channels_);
    std::vector<cv::Mat> channels;
    {
        cv::Mat frame_bin__ = frame_bin * .5;
        channels.push_back(channels_[0] + frame_bin__);
        channels.push_back(channels_[1] - frame_bin__);
        channels.push_back(channels_[2] - frame_bin__);
        cv::merge(channels, frame);
    }

    return points;
}
