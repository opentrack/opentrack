/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#ifndef POINTEXTRACTOR_H
#define POINTEXTRACTOR_H

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include "ftnoir_tracker_pt_settings.h"
#include <QMutex>

#include <vector>

class PointExtractor
{
public:
    // extracts points from frame and draws some processing info into frame, if draw_output is set
    // dt: time since last call in seconds
    // WARNING: returned reference is valid as long as object
    const std::vector<cv::Vec2f> &extract_points(cv::Mat &frame);
    int get_n_points() { QMutexLocker l(&mtx); return points.size(); }
    PointExtractor();

    settings_pt s;
private:
    static constexpr double pi = 3.14159265359;
    static constexpr int hist_c = 1;

    std::vector<cv::Vec2f> points;
    QMutex mtx;
    cv::Mat frame_gray;
    cv::Mat frame_bin;
    cv::Mat hist;

    enum { max_blobs = 16 };

    struct blob
    {
        double radius;
        cv::Vec2d pos;
        double confid;
        blob(double radius, const cv::Vec2d& pos, double confid) : radius(radius), pos(pos), confid(confid)
        {
            //qDebug() << "radius" << radius << "pos" << pos[0] << pos[1] << "confid" << confid;
        }
    };

    std::vector<blob> blobs;
    std::vector<std::vector<cv::Point>> contours;
};

#endif //POINTEXTRACTOR_H
