/* Copyright (c) 2019 Stephane Lenclud
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "settings.h"

#include "cv/numeric.hpp"
#include "options/options.hpp"
#include "video/camera.hpp"

#include <tuple>
#include <type_traits>
#include <memory>

#include <opencv2/core.hpp>

#include <QImage>
#include <QString>


const int KPointCount = 3;

class IPointExtractor
{
public:
    using vec2 = numeric_types::vec2;
    using f = numeric_types::f;

    virtual void extract_points(const cv::Mat& image, cv::Mat* aPreview, std::vector<vec2>& aPoints) = 0;
};


