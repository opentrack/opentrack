/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <unordered_map>
#include <unordered_set>
#include <tuple>
#include <memory>
#include "meanvector.h"
#include "marker-detection-info.h"
#include "marker-pose-refinement-solver.h"
#include "marker-pair-refinement-solver.h"
#include "anglecoveragetracker.h"

namespace papertracker {
    class MarkerModel {
    public:
        MarkerModel();
        void update(const std::vector<marker_detection_info> &detections);
        void build(int key_marker_id, double marker_size_cm, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs);
        bool has_marker(int id) const;
        size_t num_markers() const;
        std::vector<int> get_expected_visible_markers(const cv::Vec3d &head_rvec, const cv::Vec3d &head_tvec, const cv::Vec3d &origin_rvec, const cv::Vec3d &origin_tvec, double max_angle);
        bool solvePnP(const std::vector<marker_detection_info> &detections, const std::vector<size_t> &selected_markers, double marker_size_cm, const cv::Matx33d &camera_matrix, std::vector<double> &dist_coeffs, cv::Vec3d &rvec, cv::Vec3d &tvec);
        bool get_pose_from_single_marker(int marker_id, const cv::Vec3d &rvec_measured, const cv::Vec3d &tvec_measured, cv::Vec3d &pose_rvec, cv::Vec3d &pose_tvec);
        bool is_angle_covered(double pitch, double yaw) const;
        void set_angle_covered(double pitch, double yaw);
        void reserve(size_t count);

    private:
        struct MarkerPairObservation {
            MarkerPairObservation(int reference_marker_id, int target_marker_id) :
                reference_marker_id(reference_marker_id), target_marker_id(target_marker_id),
                rvec(MeanVector::VectorType::ROTATION),
                tvec(MeanVector::VectorType::POLAR)
            {}

            MarkerPairObservation() : MarkerPairObservation(-1, -1)
            {}

            int reference_marker_id;
            int target_marker_id;
            MeanVector rvec;
            MeanVector tvec;
        };

        struct PairHash {
            std::size_t operator()(const std::pair<int, int>& p) const {
                const auto h1 = std::hash<int>{}(p.first);
                const auto h2 = std::hash<int>{}(p.second);
                
                return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2)); // as in boost hash_combine
            }
        };

        std::unordered_map<std::pair<int, int>, MarkerPairObservation, PairHash> observations;
        std::unordered_map<int, std::vector<std::pair<int, const MarkerPairObservation*>>> adjacency;
        std::unordered_map<int, cv::Vec3d> rvecs;
        std::unordered_map<int, cv::Vec3d> tvecs;
        std::vector<int> reference_id_queue;
        std::vector<cv::Vec3d> model_vertices;
        std::unordered_set<int> model_vertex_marker_ids;
        std::vector<cv::Vec2f> temp_image_points;
        std::vector<double> temp_weights;
        AngleCoverageTracker recorded_angles;
        bool build_required;

#if CV_MAJOR_VERSION >= 5
        std::unique_ptr<WeightedMarkerPoseRefinementSolver> pose_refinement_solver;
        std::unique_ptr<MarkerPairRefinementSolver> pair_refinement_solver;
#endif

        void generate_marker_vertices(const std::vector<marker_detection_info> &detections, const std::vector<size_t> &selected_markers, double marker_size_cm);
    };
}