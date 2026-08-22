/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "meanvector.h"
#include "papertracker-util.h"
#include "config.h"
#include <opencv2/opencv.hpp>

namespace papertracker {
    MeanVector::MeanVector(VectorType type) : type(type), outliers_removed_(true), outliers_removed_std_dev(0), max_sample_count(PAPERTRACKER_MAX_VECTOR_SAMPLES)
    {}

    MeanVector::MeanVector(const cv::Vec3d &v, VectorType type) : type(type), outliers_removed_(true), outliers_removed_std_dev(0), max_sample_count(PAPERTRACKER_MAX_VECTOR_SAMPLES) {
        vectors.push_back(v);
        cached_value = v;
    }

    void MeanVector::update(const cv::Vec3d &vector) {
        if ((int)vectors.size() == max_sample_count)
            vectors.erase(vectors.begin());

        vectors.push_back(vector);

        if (type == VectorType::ROTATION)
            cached_value = average_rotation(vectors);
        else // type == VectorType::POLAR
            cached_value = average_translation(vectors);

        outliers_removed_ = false;
    }

    void MeanVector::set(const cv::Vec3d &vector) {
        vectors.clear();
        vectors.push_back(vector);
        cached_value = vector;
        outliers_removed_ = true;
        outliers_removed_std_dev = 0;
    }

    void MeanVector::remove_outliers(double std_dev_threshold) {
        if (vectors.size() < 3)
            return;

        if (type == VectorType::ROTATION) {
            std::vector<double> distances;

            distances.reserve(vectors.size());
            for (const auto &v : vectors)
                distances.push_back(angle_between_rotations(v, cached_value));

            double mean_distance = 0.0;
            for (double d : distances)
                mean_distance += d;
            mean_distance /= distances.size();

            double variance = 0.0;
            for (double d : distances)
                variance += (d - mean_distance) * (d - mean_distance);
            double std_dev = std::sqrt(variance / distances.size());

            double cutoff = mean_distance + std_dev_threshold * std_dev;

            std::vector<cv::Vec3d> pruned;
            pruned.reserve(vectors.size());

            for (size_t i = 0; i < vectors.size(); ++i)
                if (distances[i] <= cutoff)
                    pruned.push_back(vectors[i]);

            if (pruned.size() != 0) {
                if (pruned.size() < vectors.size())
                    cached_value = average_rotation(pruned);

                vectors = std::move(pruned);
            }
        } else /* type == VectorType::POLAR */ {
            std::vector<double> distances;
            distances.reserve(vectors.size());
            for (const auto &v : vectors) {
                cv::Vec3d diff = v - cached_value;
                distances.push_back(cv::norm(diff));
            }

            double mean_distance = 0.0;
            for (double d : distances)
                mean_distance += d;
            mean_distance /= distances.size();

            double variance = 0.0;
            for (double d : distances)
                variance += (d - mean_distance) * (d - mean_distance);
            double std_dev = std::sqrt(variance / distances.size());

            double cutoff = mean_distance + std_dev_threshold * std_dev;

            std::vector<cv::Vec3d> pruned;
            pruned.reserve(vectors.size());

            for (size_t i = 0; i < vectors.size(); ++i)
                if (distances[i] <= cutoff)
                    pruned.push_back(vectors[i]);

            if (pruned.size() != 0) {
                if (pruned.size() < vectors.size())
                    cached_value = average_translation(pruned);

                vectors = std::move(pruned);
            }
        }

        outliers_removed_ = true;
        outliers_removed_std_dev = std_dev_threshold;
    }

    bool MeanVector::outliers_removed(double within_std_dev) const {
        return outliers_removed_ && outliers_removed_std_dev <= within_std_dev;
    }

    int MeanVector::sample_count() const {
        return (int) vectors.size();
    }

    int MeanVector::get_max_sample_count() const {
        return max_sample_count;
    }

    const cv::Vec3d &MeanVector::get() const {
        return cached_value;
    }
}
