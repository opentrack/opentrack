/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "point_extractor.h"
#include <QDebug>


using namespace cv;
using namespace std;


PointExtractor::PointExtractor(){
	//if (!AllocConsole()){}
	//else SetConsoleTitle("debug");
	//freopen("CON", "w", stdout);
	//freopen("CON", "w", stderr);
}
// ----------------------------------------------------------------------------
std::vector<Vec2f> PointExtractor::extract_points(Mat& frame)
{
	const int W = frame.cols;
	const int H = frame.rows; 

	// convert to grayscale
	Mat frame_gray;
    cvtColor(frame, frame_gray, cv::COLOR_RGB2GRAY);
   
    int min_size = s.min_point_size;
    int max_size = s.max_point_size;
    
	unsigned int region_size_min = 3.14*min_size*min_size/4.0;
	unsigned int region_size_max = 3.14*max_size*max_size/4.0;
    
    // testing indicates threshold difference of 45 from lowest to highest
    // that's applicable to poor lighting conditions.
    
    static constexpr int diff = 20;
    static constexpr int steps = 10;
    static constexpr int successes = 8;
    
    int thres = s.threshold;
    
    struct blob {
        double max_radius;
        std::vector<cv::Vec2d> pos;
        std::vector<double> confids;
        
        cv::Vec2d effective_pos() const
        {
            double x = 0;
            double y = 0;
            double norm = 0;
            for (unsigned i = 0; i < pos.size(); i++)
            {
                x += pos[i][0] * confids[i];
                y += pos[i][1] * confids[i];
                norm += confids[i];
            }
            cv::Vec2d ret(x, y);
            ret *= 1./norm;
            //qDebug() << "ret" << ret[0] << ret[1] << "norm" << norm << "count" << pos.size();
            return ret;
        }
    };
    
    struct simple_blob
    {
        double radius;
        cv::Vec2d pos;
        double confid;
        bool taken;
        simple_blob(double radius, const cv::Vec2d& pos, double confid) : radius(radius), pos(pos), confid(confid), taken(false)
        {
            //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1] << "confid" << confid;
        }
        bool inside(const simple_blob& other)
        {
            cv::Vec2d tmp = pos - other.pos;
            double p = sqrt(1e-4 + tmp.dot(tmp));
            return p < radius;
        }
        static std::vector<blob> merge(std::vector<simple_blob>& blobs)
        {
            std::vector<blob> ret;
            for (unsigned i = 0; i < blobs.size(); i++)
            {
                auto& b = blobs[i];
                if (b.taken)
                    continue;
                b.taken = true;
                blob b_;
                b_.pos.push_back(b.pos);
                b_.confids.push_back(b.confid);
                b_.max_radius = b.radius;
                
                for (unsigned j = i+1; j < blobs.size(); j++)
                {
                    auto& b2 = blobs[j];
                    if (b2.taken)
                        continue;
                    if (b.inside(b2) || b2.inside(b))
                    {
                        b2.taken = true;
                        b_.pos.push_back(b2.pos);
                        b_.confids.push_back(b2.confid);
                        b_.max_radius = std::max(b_.max_radius, b2.radius);
                    }
                }
                if (b_.pos.size() >= successes)
                    ret.push_back(b_);
            }
            return ret;
        }
    };
    
    // mask for everything that passes the threshold (or: the upper threshold of the hysteresis)
	Mat frame_bin = cv::Mat::zeros(H, W, CV_8U);
    
    const int min = std::max(0, thres - diff/2);
    const int max = std::min(255, thres + diff/2);
    const int step = std::max(1, diff / steps);
    
    std::vector<simple_blob> blobs;
    
    // this code is based on OpenCV SimpleBlobDetector
    for (int i = min; i < max; i += step)
    {
        Mat frame_bin_;
        threshold(frame_gray, frame_bin_, i, 255, THRESH_BINARY);
        frame_bin.setTo(170, frame_bin_);
        
        std::vector<std::vector<cv::Point>> contours;
        cv::findContours(frame_bin_, contours, CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE);
        
        for (auto& c : contours)
        {
            auto m = cv::moments(cv::Mat(c));
            const double area = m.m00;
            if (area == 0.)
                continue;
            cv::Vec2d pos(m.m10 / m.m00, m.m01 / m.m00);
            if (area < region_size_min || area > region_size_max)
                continue;
            
            double radius = 0;
            
            for (auto& k : c)
            {
                cv::Vec2d pos_(k.x, k.y);
                cv::Vec2d tmp = pos_ - pos;
                radius = std::max(radius, sqrt(1e-2 + tmp.dot(tmp)));
            }
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
            blobs.push_back(simple_blob(radius, pos, confid));
        }
    }
    
    // clear old points
	points.clear();
    
    for (auto& b : simple_blob::merge(blobs))
    {
        auto pos = b.effective_pos();
        Vec2f p((pos[0] - W/2)/W, -(pos[1] - H/2)/W);
        points.push_back(p);
    }
    
    // draw output image
    vector<Mat> channels;
    channels.push_back(frame_gray + frame_bin);
    channels.push_back(frame_gray - frame_bin);
    channels.push_back(frame_gray - frame_bin);
    merge(channels, frame);

	return points;
}
