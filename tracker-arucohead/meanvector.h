/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <vector>
#include <opencv2/core.hpp>

namespace arucohead {
    class MeanVector
    {
    public:
        enum class VectorType {
            ROTATION,
            POLAR
        };

        MeanVector();
        MeanVector(const cv::Vec3d &v, VectorType type);

        void update(const cv::Vec3d &vector);
        int sample_count();
        int get_max_sample_count();
        const cv::Vec3d &get();

    private:
        std::vector<cv::Vec3d> vectors;
        cv::Vec3d cached_value;
        VectorType type;
        int max_sample_count;
    };
}