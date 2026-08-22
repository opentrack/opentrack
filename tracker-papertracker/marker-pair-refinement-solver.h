/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <opencv2/core.hpp>
#include <array>
#include <vector>
#include <unordered_map>

#if CV_MAJOR_VERSION >= 5
#include <opencv2/geometry/3d.hpp>
#include <memory>
#endif

namespace papertracker {
    class MarkerPairRefinementSolver {
    public:
        MarkerPairRefinementSolver(int max_samples_per_pair = 20);

        bool operator()(cv::InputOutputArray param, cv::OutputArray err, cv::OutputArray J);
        bool add_pair_observation(int parent_id, int child_id, const cv::Vec3d &parent_rvec, const cv::Vec3d &parent_tvec, const std::array<cv::Point2f, 4> &child_image_points, bool evict_oldest = true);
        bool refine_pair(int parent_id, int child_id, cv::Vec3d &relative_rvec, cv::Vec3d &relative_tvec, double marker_size_cm, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs);

    #if CV_MAJOR_VERSION >= 5
    private:
        struct PairHash {
            std::size_t operator()(const std::pair<int, int>& p) const {
                const auto h1 = std::hash<int>{}(p.first);
                const auto h2 = std::hash<int>{}(p.second);
                
                return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2)); // as in boost hash_combine
            }
        };

        struct PairInfo {
            cv::Vec3d parent_rvec;
            cv::Vec3d parent_tvec;
            std::array<cv::Point2f, 4> image_points;
        };

        cv::Matx33d camera_matrix;
        std::vector<double> dist_coeffs;
        double marker_size_cm;
        std::unordered_map<std::pair<int, int>, std::vector<PairInfo>, PairHash> pair_samples;
        std::pair<int, int> current_pair;
        int max_samples_per_pair;
        std::unique_ptr<cv::LevMarq> lm_solver;
        cv::Mat jacobian;
        cv::Mat jacobian_output_block;
        int max_samples_seen;
    #endif
    };
}