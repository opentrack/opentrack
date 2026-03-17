/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "meanvector.h"
#include "arucohead-util.h"
#include "config.h"

namespace arucohead {
    MeanVector::MeanVector() : type(VectorType::POLAR), max_sample_count(ARUCOHEAD_MAX_VECTOR_SAMPLES)
    {}

    MeanVector::MeanVector(const cv::Vec3d &v, VectorType type) : type(type), max_sample_count(ARUCOHEAD_MAX_VECTOR_SAMPLES) {
        vectors.push_back(v);
        cached_value = v;
    }

    void MeanVector::update(const cv::Vec3d &vector) {
        if ((int)vectors.size() >= max_sample_count)
            vectors.erase(vectors.begin());

        vectors.push_back(vector);

        if (type == VectorType::ROTATION)
            cached_value = average_rotation(vectors);
        else
            cached_value = average_translation(vectors);
    }

    int MeanVector::sample_count() {
        return (int) vectors.size();
    }

    int MeanVector::get_max_sample_count() {
        return max_sample_count;
    }

    const cv::Vec3d &MeanVector::get() {
        return cached_value;
    }
}
