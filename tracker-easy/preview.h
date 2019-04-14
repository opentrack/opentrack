/*
 * Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */


#pragma once

#include "tracker-easy-api.h"

#include <opencv2/core.hpp>
#include <QImage>

namespace EasyTracker
{

    struct Preview
    {
        Preview(int w, int h);

        Preview& operator=(const cv::Mat& frame);
        QImage get_bitmap();
        void draw_head_center(numeric_types::f x, numeric_types::f y);

        operator cv::Mat&() { return iFrameResized; }
        operator cv::Mat const&() const { return iFrameResized; }

    private:
        static void ensure_size(cv::Mat& frame, int w, int h, int type);

    public:
        cv::Mat iFrameResized, frame_out;
        cv::Mat iFrameRgb;
        cv::Mat iFrameChannelSizeOne;
    };

}

