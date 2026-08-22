/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <vector>
#include <opencv2/core.hpp>

namespace papertracker {
    class MeanVector
    {
    public:
        static constexpr double default_std_dev_threshold = 2.0;

        enum class VectorType {
            ROTATION,
            POLAR
        };

        MeanVector(VectorType type);
        MeanVector(const cv::Vec3d &v, VectorType type);

        void update(const cv::Vec3d &vector);
        void set(const cv::Vec3d &vector);
        void remove_outliers(double std_dev_threshold = default_std_dev_threshold);
        bool outliers_removed(double within_std_dev = default_std_dev_threshold) const;
        int sample_count() const;
        int get_max_sample_count() const;
        const cv::Vec3d &get() const;

    private:
        std::vector<cv::Vec3d> vectors;
        cv::Vec3d cached_value;
        bool outliers_removed_;
        double outliers_removed_std_dev;
        VectorType type;
        int max_sample_count;
    };
}