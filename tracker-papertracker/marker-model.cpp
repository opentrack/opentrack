/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "marker-model.h"
#include "papertracker-util.h"
#include "marker-pose-refinement-solver.h"
#include "config.h"

#if __has_include(<opencv2/calib3d.hpp>)
#   include <opencv2/calib3d.hpp>
#else
#   include <opencv2/calib.hpp>
#endif

namespace papertracker {
    /* Class to build and manage a spatial model of observed fiducial markers.
    */
    MarkerModel::MarkerModel() : build_required(false), recorded_angles(2.0 * CV_PI / PAPERTRACKER_MARKER_MODEL_PITCH_STEPS, 2.0 * CV_PI / PAPERTRACKER_MARKER_MODEL_YAW_STEPS) {
#if CV_MAJOR_VERSION >= 5
        pose_refinement_solver = std::make_unique<WeightedMarkerPoseRefinementSolver>(model_vertices, temp_image_points, temp_weights);
        pair_refinement_solver = std::make_unique<MarkerPairRefinementSolver>();
#endif
    }

    /* Gather translation and rotation vectors for markers B relative to markers A.
    */
    void MarkerModel::update(const std::vector<marker_detection_info> &detections) {
        for (const auto &detection_a : detections) {
            for (const auto &detection_b : detections) {
                if (detection_a.id == detection_b.id || !detection_a.solved || !detection_b.solved)
                    continue;

                const auto [rvec_local, tvec_local] = get_marker_local_transform(detection_b.rvec, detection_b.tvec, detection_a.rvec, detection_a.tvec);

                const std::pair<int, int> marker_pair = { detection_a.id, detection_b.id };

                auto observation_it = observations.find(marker_pair);

                if (observation_it == observations.end()) {
                    observation_it = observations.emplace(marker_pair, MarkerPairObservation{ detection_a.id, detection_b.id }).first;
                    adjacency[detection_a.id].emplace_back(detection_b.id, &observation_it->second);
                }

                observation_it->second.rvec.update(rvec_local);
                observation_it->second.tvec.update(tvec_local);

#if CV_MAJOR_VERSION >= 5
                pair_refinement_solver->add_pair_observation(detection_a.id, detection_b.id, detection_a.rvec, detection_a.tvec, detection_b);
#endif
            }
        }

        build_required = true;
    }

    /* Build a representation of the marker set where the key marker is at the origin.
    */
    void MarkerModel::build(int key_marker_id, double marker_size_cm, const cv::Matx33d &camera_matrix, const std::vector<double> &dist_coeffs) {
        if (!build_required)
            return;

        rvecs.clear();
        tvecs.clear();

        rvecs[key_marker_id] = cv::Vec3d();
        tvecs[key_marker_id] = cv::Vec3d();

        reference_id_queue.clear();
        reference_id_queue.push_back(key_marker_id);

        size_t next_reference_index = 0;

        while (next_reference_index < reference_id_queue.size()) {
            const int reference_id = reference_id_queue[next_reference_index++];

            const auto adjacency_it = adjacency.find(reference_id);
            if (adjacency_it == adjacency.end())
                continue;

            for (const auto &[target_id, observation] : adjacency_it->second) {
                if (rvecs.count(target_id) != 0)
                    continue;

                auto rvec_local = observation->rvec.get();
                auto tvec_local = observation->tvec.get();

#if CV_MAJOR_VERSION >= 5
                pair_refinement_solver->refine_pair(observation->reference_marker_id, observation->target_marker_id, rvec_local, tvec_local, marker_size_cm, camera_matrix, dist_coeffs);
#endif

                const auto rvec_parent = rvecs[reference_id];
                const auto tvec_parent = tvecs[reference_id];

                cv::Matx33d R_reference;
                cv::Rodrigues(rvec_parent, R_reference);

                cv::Matx33d R_local;
                cv::Rodrigues(rvec_local, R_local);

                cv::Vec3d world_rvec;
                cv::Rodrigues(R_reference * R_local, world_rvec);

                const auto world_tvec = R_reference * tvec_local + tvec_parent;

                rvecs[target_id] = world_rvec;
                tvecs[target_id] = world_tvec;

                reference_id_queue.push_back(target_id);
            }
        }

        build_required = false;
    }

    /* Determine if the model contains given marker.
    */
    bool MarkerModel::has_marker(int id) const {
        return rvecs.count(id) > 0;
    }

    /* Obtain the number of markers included in the model.
    */
    size_t MarkerModel::num_markers() const {
        return rvecs.size();
    }

    /* Set an offset for marker pair image points to account for changes to camera zoom setting.
    */
    void MarkerModel::set_image_point_offset(const cv::Vec2f &image_point_offset) {
#if CV_MAJOR_VERSION >= 5
        pair_refinement_solver->set_image_point_offset(image_point_offset);
#endif
    }

    /* Get a list of markers expected to be facing the camera at a maximum  angle of max_angle for the given head pose.
    */
    std::vector<int> MarkerModel::get_expected_visible_markers(const cv::Vec3d &head_rvec, const cv::Vec3d &head_tvec, const cv::Vec3d &origin_rvec, const cv::Vec3d &origin_tvec, double max_angle) {
        std::vector<int> visible;

        cv::Matx33d R_origin;
        cv::Rodrigues(origin_rvec, R_origin);

        cv::Matx33d R_head;
        cv::Rodrigues(head_rvec, R_head);

        for (const auto &[marker_id, marker_rvec] : rvecs) {
            cv::Matx33d R_marker;
            cv::Rodrigues(marker_rvec, R_marker);

            cv::Matx33d R_composite = R_head * R_origin * R_marker;

            cv::Vec3d marker_direction = head_tvec + R_head * (origin_tvec + tvecs[marker_id]);
            marker_direction = marker_direction / -cv::norm(marker_direction);

            const double angle = get_marker_relative_angle(R_composite, marker_direction);

            if (angle <= max_angle)
                visible.push_back(marker_id);
        }

        return visible;
    }

    /*  Compute pose for the marker model from a list of selected markers as a single unit.
    */
    bool MarkerModel::solvePnP(const std::vector<marker_detection_info> &detections, const std::vector<size_t> &selected_markers, double marker_size_cm, const cv::Matx33d &camera_matrix, std::vector<double> &dist_coeffs, cv::Vec3d &rvec, cv::Vec3d &tvec) {
        generate_marker_vertices(detections, selected_markers, marker_size_cm);

        if (model_vertices.size() == 0)
            return false;

        temp_image_points.clear();
        temp_weights.clear();

        for (auto i : selected_markers) {
            if (model_vertex_marker_ids.count(detections[i].id) > 0) {
                temp_image_points.insert(temp_image_points.end(), detections[i].begin(), detections[i].end());
                temp_weights.push_back(detections[i].weight);
            }
        }

#if CV_MAJOR_VERSION >= 5
        // Initial solution.
        if (!cv::solvePnP(model_vertices, temp_image_points, camera_matrix, dist_coeffs, rvec, tvec, false, cv::SOLVEPNP_ITERATIVE))
            return false;

        // Note to maintainers: the choice of SOLVEPNP_ITERATIVE above is deliberate in spite of the subsequent refinement
        // step. Other methods I've tried produce results that Levenberg-Marquardt refinement can't fix. The extra
        // refinement step is to apply marker weights, specifically, rather than for overall refinement.

        // Apply marker weights via pose refinement.
        pose_refinement_solver->refineLeastSquares(rvec, tvec, camera_matrix, dist_coeffs);

#else
        if (!cv::solvePnP(model_vertices, temp_image_points, camera_matrix, dist_coeffs, rvec, tvec, false, cv::SOLVEPNP_ITERATIVE))
            return false;
#endif

        return true;
    }

    bool MarkerModel::get_pose_from_single_marker(int marker_id, const cv::Vec3d &rvec_measured, const cv::Vec3d &tvec_measured, cv::Vec3d &pose_rvec, cv::Vec3d &pose_tvec) {
        if (rvecs.count(marker_id) == 0)
            return false;

        const cv::Vec3d &marker_local_rvec = rvecs[marker_id];
        const cv::Vec3d &marker_local_tvec = tvecs[marker_id];

        cv::Matx33d R_marker_local;
        cv::Rodrigues(marker_local_rvec, R_marker_local);

        cv::Matx33d R_measured;
        cv::Rodrigues(rvec_measured, R_measured);

        const cv::Matx33d R_pose = R_measured * R_marker_local.t();
        cv::Rodrigues(R_pose, pose_rvec);

        pose_tvec = tvec_measured - R_pose * marker_local_tvec;

        return true;
    }


    bool MarkerModel::is_angle_covered(double pitch, double yaw) const {
        return recorded_angles.get_visit_count(pitch, yaw) > 0;
    }

    void MarkerModel::set_angle_covered(double pitch, double yaw) {
        const auto bin = recorded_angles.get_bin(pitch, yaw);

        if (recorded_angles.get_visit_count(bin) == 0)
            recorded_angles.add_visit(bin);
    }

    /* Reserve enough space for count markers and a suitable number of observation pairs.
    */
    void MarkerModel::reserve(size_t count) {
        rvecs.reserve(count);
        tvecs.reserve(count);
        reference_id_queue.reserve(count);
        model_vertices.reserve(count * 4);
        model_vertex_marker_ids.reserve(count);
        adjacency.reserve(count);

        if (count > 0)
            observations.reserve(count * (count - 1));

        temp_image_points.reserve(count * 4);
        temp_weights.reserve(count);
    }

    /* Generate a list of vertices for markers in detections as indexed by selected_markers.
       Vertex order will be the same as in selected_markers. An std::set is also created
       indicating which of the requested marker IDs (if any) are represented in the list.
    */
    void MarkerModel::generate_marker_vertices(
            const std::vector<marker_detection_info> &detections,
            const std::vector<size_t> &selected_markers,
            double marker_size_cm) 
        {
        const std::array<cv::Vec3d, 4> objectPoints = {{
            { -marker_size_cm / 2.0,  marker_size_cm / 2.0, 0.0 },
            { marker_size_cm / 2.0,  marker_size_cm / 2.0, 0.0 },
            { marker_size_cm / 2.0, -marker_size_cm / 2.0, 0.0 },
            { -marker_size_cm / 2.0, -marker_size_cm / 2.0, 0.0 }
        }};

        model_vertices.clear();
        model_vertex_marker_ids.clear();

        for (const auto marker_index : selected_markers) {
            const int marker_id = detections[marker_index].id;

            if (rvecs.count(marker_id) == 0)
                continue;

            std::array<cv::Vec3d, 4> translatedPoints;

            cv::Matx33d R;
            cv::Rodrigues(rvecs[marker_id], R);

            const auto tvec = tvecs[marker_id];

            cv::Matx34d T(
                R(0,0), R(0,1), R(0,2), tvec[0],
                R(1,0), R(1,1), R(1,2), tvec[1],
                R(2,0), R(2,1), R(2,2), tvec[2]
            );

            cv::transform(objectPoints, translatedPoints, T);

            model_vertices.push_back(translatedPoints[0]);
            model_vertices.push_back(translatedPoints[1]);
            model_vertices.push_back(translatedPoints[2]);
            model_vertices.push_back(translatedPoints[3]);

            model_vertex_marker_ids.insert(marker_id);
        }
    }
}
