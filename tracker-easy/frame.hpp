#pragma once

#include "tracker-easy-api.h"

#include <opencv2/core.hpp>
#include <QImage>



struct Preview
{
    Preview(int w, int h);

    Preview& operator=(const cv::Mat& frame);
    QImage get_bitmap();
    void draw_head_center(Coordinates::f x, Coordinates::f y);

    operator cv::Mat&() { return frame_copy; }
    operator cv::Mat const&() const { return frame_copy; }

private:
    static void ensure_size(cv::Mat& frame, int w, int h, int type);

    cv::Mat frame_copy, frame_out;
};


