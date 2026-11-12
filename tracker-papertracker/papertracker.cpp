/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "papertracker.h"
#include "papertracker-util.h"
#include "marker-detect.h"
#include "api/plugin-api.hpp"
#include "cv/init.hpp"
#include "compat/sleep.hpp"
#if __has_include(<opencv2/calib3d.hpp>)
#   include <opencv2/calib3d.hpp>
#else
#   include <opencv2/calib.hpp>
#endif
#include <opencv2/imgproc.hpp>
#include "config.h"
#include <unordered_map>
#include <unordered_set>
#include <limits>
#include <QDebug>

using namespace papertracker;

/* Open camera selected in settings.
*/
bool PaperTracker::open_camera()
{
    /* Prevent concurrent access to the camera.
    */
    QMutexLocker l(&camera_mtx);

    /* Create camera object.
    */
    QString camera_name = static_settings.camera_name;

    if (static_settings.camera_name == "") {
        const auto camera_names = video::camera_names();

        if (camera_names.size() == 0)
            return false;

        camera_name = camera_names[0];
    }

    camera = video::make_camera(camera_name);
    if (!camera)
        return false;

    /* Start camera.
    */
    video::impl::camera::info args {};
    args.width = static_settings.frame_width;
    args.height = static_settings.frame_height;
    args.fps = static_settings.fps;
    args.use_mjpeg = static_settings.use_mjpeg;

    if (!camera->start(args)) {
        qDebug() << "PaperTracker: could not open camera.";
        return false;
    }

    return true;
}

/* Detect markers, update head pose, and draw AR elements.
*/
bool PaperTracker::process_frame(cv::Mat &frame, const cv::Rect2i *roi)
{
    /* Clear data from last frame.
    */
    frame_data.excluded_markers.clear();
    frame_data.selected_markers.clear();

    /* Marker corners (in object space).
    */
    const double marker_size_cm = static_settings.aruco_marker_size_mm / 10.0;

    const cv::Matx43d objectPoints = {
        -marker_size_cm / 2.0,  marker_size_cm / 2.0, 0.0,
         marker_size_cm / 2.0,  marker_size_cm / 2.0, 0.0,
         marker_size_cm / 2.0, -marker_size_cm / 2.0, 0.0,
        -marker_size_cm / 2.0, -marker_size_cm / 2.0, 0.0
    };

    /* Create ROI image.
    */
    cv::Rect2i roi_copy;
    cv::Mat image;

    if (!has_key_marker)
        roi = nullptr;

    if (roi) {
        roi_copy = *roi;

        if (roi->width > 0 && roi->height > 0)
        {
            auto intersection = *roi & cv::Rect2i(0, 0, frame.cols, frame.rows);

            if (intersection.width > 0 && intersection.height > 0) {
                image = frame(intersection);
                roi_copy = intersection;
                roi = &roi_copy;
            } else {
                image = frame;
                roi = nullptr;
            }
        } else {
            image = frame;
            roi = nullptr;
        }
    } else {
        image = frame;
    }

    /* Detect markers.
    */
    frame_data.returned_markers.clear();

    if (QMutexLocker l(&settings_update_mtx); true) {
        /* Until the key marker has been established, use the cycled thresholding parameters and search for
           optimal ones regardless of contrast method. Afterwards, use the configured contrast method.
        */
        if (!has_key_marker) {
            set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);
            detect_markers_optimal(image, frame_data.returned_markers);
        } else {
            apply_contrast_method();
            detector.detect(image, frame_data.returned_markers, cv::Mat(), cv::Mat(), -1, false);
        }
    }

    const int min_marker_id = s.first_marker_id;
    const int max_marker_id = s.first_marker_id + s.number_of_markers - 1;

    detected_markers.clear();
    for (const auto &marker : frame_data.returned_markers) {
        if (marker.id < min_marker_id || marker.id > max_marker_id)
            continue;

        detected_markers.push_back(marker_detection_info(marker.id, marker));
    }

    /* Track how many consecutive frames each marker has been detected for.
    */
    update_marker_streaks(detected_markers);

    /* Express corners in terms of original frame, if necessary.
    */
    if (roi != nullptr) {
        for (auto &marker : detected_markers) {
            for (auto &corner : marker) {
                corner.x += roi->x;
                corner.y += roi->y;
            }
        }
    }

    /* If no markers detected, note it as a failed detection.
    */
    bool detection_failed = detected_markers.empty();

    if (detection_failed && roi != nullptr)
        return false; // ROI needs to be reset, so return immediately.

    last_roi = get_marker_detected_region(detected_markers);

    /* Caclulate poses for detected markers.
    */
    const double min_angle_radians = s.marker_min_angle * CV_PI / 180.0;
    const double max_angle_radians = s.marker_max_angle * CV_PI / 180.0;
    const double angle_range = max_angle_radians - min_angle_radians;
    const double fadeout_range = PAPERTRACKER_MARKER_ANGLE_FADEOUT_RANGE_PERCENT * angle_range / 2.0;
    const double sort_reference_angle = min_angle_radians + fadeout_range;

    for (size_t i = 0; i < detected_markers.size(); ++i) {
        if (cv::solvePnPGeneric(objectPoints, detected_markers[i], camera_matrix, dist_coeffs, frame_data.pnp_rvecs, frame_data.pnp_tvecs, false, cv::SOLVEPNP_IPPE_SQUARE, cv::noArray(), cv::noArray(), frame_data.pnp_reprojection_errors)) {
            const int id = detected_markers[i].id;

            /* Choose the best solution among the two candidates. */
            const double error0 = frame_data.pnp_reprojection_errors[0];
            const double error1 = frame_data.pnp_reprojection_errors[1];
            const double error_ratio = std::min(error0, error1) / std::max(error0, error1);

            const int solution_index = error0 <= error1 ? 0 : 1;

            detected_markers[i].rvec = frame_data.pnp_rvecs[solution_index];
            detected_markers[i].tvec = frame_data.pnp_tvecs[solution_index];
            detected_markers[i].solved = true;

            /* Set marker weight. Markers near configured angle limits get a lower weight, as do markers having a
               low stability score (based on the number of frames a marker has been seen consistently).
            */
            const auto marker_direction = -detected_markers[i].tvec / cv::norm(detected_markers[i].tvec);
            detected_markers[i].z_angle = get_marker_relative_angle(detected_markers[i].rvec, marker_direction);

            const double fade_low = std::clamp((detected_markers[i].z_angle - min_angle_radians) / fadeout_range, 0.0, 1.0);
            const double fade_high = std::clamp((max_angle_radians - detected_markers[i].z_angle) / fadeout_range, 0.0, 1.0);
            const double stability = std::clamp(get_marker_stability_score(id), 0.0, 1.0);

            detected_markers[i].weight = fade_low * fade_high * stability;

            /* Exclude markers that are too ambiguous, too close to head-on, or too oblique.
            */
            if (error_ratio > PAPERTRACKER_MARKER_EXCLUSION_AMBIGUITY_THRESHOLD ||
                detected_markers[i].z_angle < min_angle_radians ||
                detected_markers[i].z_angle > max_angle_radians)
            {
                frame_data.excluded_markers.insert(id);
            }
        }
    }

    /* Prune away unreliable markers unless none would remain.
     */
    marker_highlight_set.clear();

    const size_t excluded_marker_count = frame_data.excluded_markers.size();

    if (has_key_marker && excluded_marker_count > 0) {
        if (excluded_marker_count < detected_markers.size()) {
            /* Keep only reliable markers.
            */
            for (size_t i = 0; i < detected_markers.size(); ++i) {
                if (frame_data.excluded_markers.count(detected_markers[i].id) == 0) {
                    frame_data.selected_markers.push_back(i);
                    marker_highlight_set.insert(detected_markers[i].id);
                }
            }
        } else if (detected_markers.size() > 0) {
            /* Fallback: no good markers, so choose among the remaining markers.
            */
            std::sort(detected_markers.begin(), detected_markers.end(), [this, sort_reference_angle](const auto &a, const auto &b) {
                const bool model_has_a = marker_model.has_marker(a.id);
                const bool model_has_b = marker_model.has_marker(b.id);

                if (model_has_a && !model_has_b)
                    return true;
                else if (!model_has_a && model_has_b)
                    return false;

                const double sorting_angle_a = fabs(a.z_angle - sort_reference_angle);
                const double sorting_angle_b = fabs(b.z_angle - sort_reference_angle);

                return sorting_angle_a < sorting_angle_b;
            });

            /* Use the first marker.
            */
            frame_data.selected_markers.push_back(0);
            marker_highlight_set.insert(detected_markers[0].id);
        }
    } else {
        for (size_t i = 0; i < detected_markers.size(); ++i) {
            frame_data.selected_markers.push_back(i);
            marker_highlight_set.insert(detected_markers[i].id);
        }
    }

    /* Find key marker if it has not yet been detected.
    */
    if (!has_key_marker) {
        const auto key_markers = get_key_markers(detected_markers);

        if (key_markers.size() > 0) {
            // Set the key marker.
            for (const auto marker_index : key_markers) {
                starting_rvecs.push_back(detected_markers[marker_index].rvec);
                starting_tvecs.push_back(detected_markers[marker_index].tvec);
            }

            key_marker_id = detected_markers[key_markers[0]].id;

            auto head_orientation = cv::Vec3d(CV_PI, 0, 0);
            auto head_origin = get_approximate_head_origin(starting_rvecs, starting_tvecs);

            auto [rvec_local, tvec_local] = get_marker_local_transform(starting_rvecs[0], starting_tvecs[0], head_orientation, head_origin);

            key_marker_local_rvec = rvec_local;
            key_marker_local_tvec = tvec_local;

            if (QMutexLocker l(&data_mtx); true) {
                starting_head_origin = head_origin;
                current_head_origin = head_origin;
            }

            has_key_marker = true;

            // Initialize the marker model.
            update_marker_model_at_pose(head_orientation, detected_markers, 1, marker_size_cm);
        }
    } else if (last_head_circumference_cm != s.head_circumference_cm || last_marker_height_cm != s.marker_height_cm) {
        // update marker origin
        auto head_orientation = cv::Vec3d(CV_PI, 0, 0);
        auto head_origin = get_approximate_head_origin(starting_rvecs, starting_tvecs);

        auto [rvec_local, tvec_local] = get_marker_local_transform(starting_rvecs[0], starting_tvecs[0], head_orientation, head_origin);

        key_marker_local_tvec = tvec_local;

        last_head_circumference_cm = s.head_circumference_cm;
        last_marker_height_cm = s.marker_height_cm;

        if (QMutexLocker l(&data_mtx); true) {
            current_head_origin = head_origin;
        }
    }

    if (has_key_marker && !frame_data.selected_markers.empty()) {
        /* Build marker model.
        */
        marker_model.build(key_marker_id, marker_size_cm, camera_matrix, dist_coeffs);

        bool head_pose_solved = false;
        bool model_updated = false;

        #if CV_MAJOR_VERSION >= 5

        /*  Compute pose using selected markers in a single call to solvePnP.
        */
        cv::Vec3d new_head_rvec;
        cv::Vec3d new_head_tvec;

        cv::Vec3d rvec;
        cv::Vec3d tvec;
        if (marker_model.solvePnP(detected_markers, frame_data.selected_markers, marker_size_cm, camera_matrix, dist_coeffs, rvec, tvec)) {
            auto [new_head_rvec, new_head_tvec] = get_head_pose_from_marker_transform(rvec, tvec);

            /* Fall back to individual marker averaging if the combined solve disagrees significantly with observed marker poses.
            */
            const double transformation_error = marker_model.transformation_error(new_head_rvec, new_head_tvec, key_marker_local_rvec, key_marker_local_tvec, detected_markers, frame_data.selected_markers, marker_size_cm);

            if (transformation_error > PAPERTRACKER_MARKER_MODEL_TRANSFORMATION_ERROR_THRESHOLD_CM) {
                /* The combined solve disagrees significantly with the observations. This can happen when the model is
                   inaccurate, causing solvePnP() to produce a worse pose than individual markers. Choose those individual
                   markers that best explain all of the selected markers and average their poses together. If the error
                   persists across several frames, update the model (up to MAX_CORRECTIVE_SAMPLES times).
                */
                consecutive_transformation_error_frames++;

                double min_error = transformation_error;

                frame_data.temp_rvecs.clear();
                frame_data.temp_tvecs.clear();
                frame_data.temp_weights.clear();

                double summed_weights = 0;
                for (const auto marker_index : frame_data.selected_markers) {
                    const auto &marker = detected_markers[marker_index];

                    if (!marker.solved)
                        continue;

                    if (!marker_model.get_pose_from_single_marker(marker.id, marker.rvec, marker.tvec, rvec, tvec))
                        continue;

                    const auto [pose_rvec, pose_tvec] = get_head_pose_from_marker_transform(rvec, tvec);

                    const double pose_error = marker_model.transformation_error(pose_rvec, pose_tvec, key_marker_local_rvec, key_marker_local_tvec, detected_markers, frame_data.selected_markers, marker_size_cm);

                    // Keep track of the pose with the smallest error to use as a fallback if no good poses found.
                    if (pose_error < min_error) {
                        min_error = pose_error;
                        new_head_rvec = pose_rvec;
                        new_head_tvec = pose_tvec;
                    }

                    // Add poses below the error threshold to the list of poses to be averaged.
                    if (pose_error < PAPERTRACKER_MARKER_MODEL_TRANSFORMATION_ERROR_THRESHOLD_CM) {
                        frame_data.temp_rvecs.push_back(pose_rvec);
                        frame_data.temp_tvecs.push_back(pose_tvec);
                        frame_data.temp_weights.push_back(marker.weight);
                        summed_weights += marker.weight;
                    }
                }

                /* Compute average of selected poses. If none have been selected, the single pose with the smallest
                   transformation error will be used instead (from previously set values).
                */
                if (!frame_data.temp_rvecs.empty()) {
                    const std::vector<double> *weights = nullptr;

                    if (summed_weights > 0.0) {
                        for (auto &weight : frame_data.temp_weights)
                            weight /= summed_weights;

                        weights = &frame_data.temp_weights;
                    }

                    new_head_rvec = average_rotation(frame_data.temp_rvecs, weights);
                    new_head_tvec = average_translation(frame_data.temp_tvecs, weights);
                }

                if (consecutive_transformation_error_frames >= PAPERTRACKER_MARKER_MODEL_CORRECTION_HYSTERESIS_FRAMES) {
                    /*  Address persistent errors by updating the model with this frame's observations. Testing reveals this will often serve to
                        repair the model. The effect may not be immediate and may require samples from other angles. Requiring the error to
                        persist guards against correcting an accurate model due to a few bad observations. Only markers
                        whose stability score has reached the stability threshold are used.
                    */
                    const auto &stable_markers = get_stable_markers(detected_markers);

                    if (stable_markers.size() >= 2)
                        model_updated = update_marker_model_at_pose(new_head_rvec, stable_markers, PAPERTRACKER_MARKER_MODEL_MAX_CORRECTIVE_SAMPLES + 1, marker_size_cm);

                    consecutive_transformation_error_frames = 0;
                }
            } else {
                consecutive_transformation_error_frames = 0;
            }

            if (QMutexLocker l(&data_mtx); true) {
                head_rvec = new_head_rvec;
                head_tvec = new_head_tvec;
            }

            head_pose_solved = true;
        }

        #else // CV_MAJOR_VERSION < 5

        /* Compute averaged pose using selected markers.
        */
        frame_data.temp_rvecs.clear();
        frame_data.temp_tvecs.clear();
        frame_data.temp_weights.clear();

        double summed_weights = 0;
        for (auto marker_index : frame_data.selected_markers) {
            auto &marker = detected_markers[marker_index];

            if (!marker.solved)
                continue;

            cv::Vec3d rvec;
            cv::Vec3d tvec;
            if (marker_model.get_pose_from_single_marker(marker.id, marker.rvec, marker.tvec, rvec, tvec)) {
                auto [pose_rvec, pose_tvec] = get_head_pose_from_marker_transform(rvec, tvec);

                frame_data.temp_rvecs.push_back(pose_rvec);
                frame_data.temp_tvecs.push_back(pose_tvec);

                frame_data.temp_weights.push_back(marker.weight);
                summed_weights += marker.weight;
            }
        }

        if (!frame_data.temp_rvecs.empty()) {
            const std::vector<double> *weights = nullptr;

            if (summed_weights > 0.0) {
                for (auto &weight : frame_data.temp_weights)
                    weight /= summed_weights;

                weights = &frame_data.temp_weights;
            }

            const auto weighted_rvec = average_rotation(frame_data.temp_rvecs, weights);
            const auto weighted_tvec = average_translation(frame_data.temp_tvecs, weights);

            if (QMutexLocker l(&data_mtx); true) {
                head_rvec = weighted_rvec;
                head_tvec = weighted_tvec;
            }

            head_pose_solved = true;
        }

        #endif

        /* Update marker model with this frame's observations if the current head pose falls in an unvisited angle bin,
           unless the corrective path above already added this frame's observations. Only markers whose stability score
           has reached the stability threshold are used, so that markers flickering in and out of detection don't
           corrupt the model. A stable marker that isn't yet part of the model is admitted regardless of whether
           the angle bin has already been visited, so that it doesn't have to wait for the next unvisited bin.
        */
        if (head_pose_solved && !model_updated) {
            const auto &stable_markers = get_stable_markers(detected_markers);

            if (stable_markers.size() >= 2) {
                bool has_new_marker = false;

                for (const auto &marker : stable_markers) {
                    if (!marker_model.has_marker(marker.id)) {
                        has_new_marker = true;
                        break;
                    }
                }

                // Always add new markers regardless of the number of visits for the current angle.
                const int max_visits = has_new_marker ? std::numeric_limits<int>::max() : 1;

                update_marker_model_at_pose(head_rvec, stable_markers, max_visits, marker_size_cm);
            }
        }

        /* Check detected markers against expectations and signal need to reset ROI if these don't match.
        */
        cv::Matx33d R;
        cv::Rodrigues(head_rvec, R);
        const auto euler = rotation_matrix_to_euler_zyx(R);

        const auto bin = visited_angles.get_bin(euler[0], euler[1]);

        if (bin != last_bin && visited_angles.get_visit_count(last_bin) < PAPERTRACKER_ANGLE_COVERAGE_VISIT_THRESHOLD)
            visited_angles.clear_visits(last_bin);

        last_bin = bin;

        if (roi == nullptr && visited_angles.get_visit_count(bin) < PAPERTRACKER_ANGLE_COVERAGE_VISIT_THRESHOLD)
            visited_angles.add_visit(bin);

        if (marker_model.num_markers() < s.number_of_markers && visited_angles.get_visit_count(bin) < PAPERTRACKER_ANGLE_COVERAGE_VISIT_THRESHOLD) {
            detection_failed = true;
        } else {
            auto expected_ids = marker_model.get_expected_visible_markers(head_rvec, head_tvec, key_marker_local_rvec, key_marker_local_tvec, CV_PI / 180.0 * s.marker_max_angle);

            if (markers_disappeared(expected_ids, detected_markers))
                detection_failed = true;
        }
    }

    return !detection_failed;
}

/* Cycle through thresholding parameters. Adapted from ftnoir_tracker_aruco.cpp.
*/
void PaperTracker::cycle_threshold_params()
{
    if (!use_fixed_threshold) {
        use_fixed_threshold = true;
    } else {
        use_fixed_threshold = false;

        adaptive_size_pos++;
        adaptive_size_pos %= std::size(adaptive_threshold_sizes);
    }

    set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);

    qDebug() << "aruco: switched thresholding params"
             << "fixed:" << use_fixed_threshold
             << "size:" << adaptive_threshold_sizes[adaptive_size_pos];
}

/* Set detector thresholding parameters explicitly. Adapted from ftnoir_tracker_aruco.cpp.
*/
void PaperTracker::set_threshold_params(bool fixed_threshold, int adaptive_size)
{
    detector.setDesiredSpeed(3);

    if (fixed_threshold)
        detector._thresMethod = aruco::MarkerDetector::FIXED_THRES;
    else
        detector._thresMethod = aruco::MarkerDetector::ADPT_THRES;

    detector.setThresholdParams(adaptive_size, PAPERTRACKER_ADAPTIVE_THRESHOLD_C);
}

/* Apply thresholding parameters for the configured contrast method.
*/
void PaperTracker::apply_contrast_method()
{
    const papertracker_contrast_method method = s.contrast_method;

    switch (method) {
    case PAPERTRACKER_CONTRAST_METHOD_GLOBAL:
        set_threshold_params(true, adaptive_threshold_sizes[0]);
        break;
    case PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_5_5:
        set_threshold_params(false, 5);
        break;
    case PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_7_7:
        set_threshold_params(false, 7);
        break;
    case PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_9_9:
        set_threshold_params(false, 9);
        break;
    case PAPERTRACKER_CONTRAST_METHOD_ADAPTIVE_11_11:
        set_threshold_params(false, 11);
        break;
    case PAPERTRACKER_CONTRAST_METHOD_LOCKED_AUTO:
    case PAPERTRACKER_CONTRAST_METHOD_CONTINUOUS_AUTO:
    default:
        // use settings from last time either auto mode was run
        set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);
        break;
    }
}

/* Detect markers using threshold parameters that maximize the number of detected markers. This is especially
   important during initial detection as lighting conditions can sometimes result in missed markers and we
   depend on context to determine the initial head position.
*/
void PaperTracker::detect_markers_optimal(const cv::Mat &image, std::vector<aruco::Marker> &best_markers)
{
    // Detect markers using current threshold parameters.
    detector.detect(image, best_markers, cv::Mat(), cv::Mat(), -1, false);

    // Set current threshold parameters as current best.
    auto best_threshold_fixed = use_fixed_threshold;
    auto best_adaptive_size_pos = adaptive_size_pos;

    // Search is slow, so to minimize the impact on frame rate only proceed if we've found at least one marker
    // using current parameters. If none are detected the parameters will be cycled normally elsewhere.
    if (best_markers.size() == 0) return;

    // Skip current threshold parameters in detections that follow.
    const auto skip_fixed_threshold = use_fixed_threshold;
    const auto skip_adaptive_size_pos = adaptive_size_pos;

    // Try fixed threshold.
    if (!skip_fixed_threshold) {
        use_fixed_threshold = true;
        set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);

        detector.detect(image, frame_data.temp_markers, cv::Mat(), cv::Mat(), -1, false);

        if (frame_data.temp_markers.size() > best_markers.size()) {
            best_markers.assign(frame_data.temp_markers.begin(), frame_data.temp_markers.end());
            best_threshold_fixed = true;
        }
    }

    // Try adaptive thresholds.
    use_fixed_threshold = false;
    for (size_t i = 0; i < std::size(adaptive_threshold_sizes); ++i) {
        if (!skip_fixed_threshold && skip_adaptive_size_pos == static_cast<int>(i))
            continue; // already tried this combination

        adaptive_size_pos = static_cast<int>(i);
        set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);

        detector.detect(image, frame_data.temp_markers, cv::Mat(), cv::Mat(), -1, false);

        if (frame_data.temp_markers.size() > best_markers.size()) {
            best_markers.assign(frame_data.temp_markers.begin(), frame_data.temp_markers.end());
            best_threshold_fixed = false;
            best_adaptive_size_pos = adaptive_size_pos;
        }
    }

    use_fixed_threshold = best_threshold_fixed;
    adaptive_size_pos = best_adaptive_size_pos;
    set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);
}

/* Generate a camera matrix for the given image dimension and field of view (in radians).
*/
cv::Matx33d PaperTracker::build_camera_matrix(int image_width, int image_height, double diagonal_fov)
{
    double diagonal_px = cv::sqrt(image_width*image_width + image_height*image_height);

    double focal_length = (diagonal_px / 2) / tan(diagonal_fov / 2);

    double cx = image_width / 2;
    double cy = image_height / 2;

    double data[9] = {
        focal_length, 0,            cx,
        0,            focal_length, cy,
        0,            0,            1
    };

    return cv::Matx33d(data);
}

/* Get the marker or markers closest to the bottom center of the current marker setup as observed by the camera.
*/
std::vector<size_t> PaperTracker::get_key_markers(const std::vector<marker_detection_info> &detection_info)
{
    if (detection_info.size() == 0)
        return {};

    // Find line of vertical symmetry.
    const auto middle = get_marker_line_of_symmetry(
        {detection_info.begin(), detection_info.end()}
    );

    // Find bottom line of markers.
    double bottom = std::numeric_limits<double>::lowest();
    for (const auto marker : detection_info)
        if (marker.solved && marker.tvec.val[1] > bottom)
            bottom = marker.tvec.val[1];

    std::vector<size_t> row_markers;
    for (size_t i = 0; i < detection_info.size(); ++i) {
        const int marker_id = detection_info[i].id;
        if (detection_info[i].solved && fabs(detection_info[i].tvec.val[1] - bottom) < static_settings.aruco_marker_size_mm / 10.0 / 2.0)
            row_markers.push_back(i);
    }

    // Return empty vector if no suitable markers found.
    if (row_markers.size() == 0)
        return {};

    // Sort markers by how close they are to the line of symmetry.
    std::sort(row_markers.begin(), row_markers.end(), [detection_info, middle](const int a, const int b) {
        float a_mid = 0;
        float b_mid = 0;

        for (const auto &corner : detection_info[a])
            a_mid += corner.x;

        for (const auto &corner : detection_info[b])
            b_mid += corner.x;

        a_mid /= detection_info[a].size();
        b_mid /= detection_info[b].size();

        return fabs(a_mid - middle) < fabs(b_mid - middle);
    });

    std::vector<size_t> key_markers;

    const auto marker = detection_info[row_markers[0]];
    if (row_markers.size() == 1 || vertical_line_intersects_marker(middle, marker)) {
        // central marker
        key_markers.push_back(row_markers[0]);
    } else {
        // two middle markers
        const size_t a = row_markers.size() / 2 - 1;
        const size_t b = a + 1;

        key_markers.push_back(row_markers[a]);
        key_markers.push_back(row_markers[b]);
    }

    return key_markers;
}

/* Get the head's starting position from the orientation and position of one or two markers. These markers
   are assumed to follow the curvature of the head.
*/
cv::Vec3d PaperTracker::get_approximate_head_origin(const std::vector<cv::Vec3d> &marker_rvecs, const std::vector<cv::Vec3d> &marker_tvecs)
{
    if (marker_rvecs.size() == 1) {
        // single marker
        const auto v = get_xz_direction_vector(marker_rvecs[0]);
        const auto c = circle_edge_intersection(circumference_to_radius(s.head_circumference_cm), v);

        const double median_cephalic_index = 0.8;
        const auto handle_offset = cv::Vec3d(c[0] * median_cephalic_index, s.marker_height_cm, c[1]);
        const auto head_tvec = marker_tvecs[0] + handle_offset;

        return head_tvec;
    } else {
        // average results between two markers
        const auto va = get_xz_direction_vector(marker_rvecs[0]);
        const auto vb = get_xz_direction_vector(marker_rvecs[1]);

        const auto ca = circle_edge_intersection(circumference_to_radius(s.head_circumference_cm), va);
        const auto cb = circle_edge_intersection(circumference_to_radius(s.head_circumference_cm), vb);

        const auto handle_offset_a = cv::Vec3d(ca[0], s.marker_height_cm, ca[1]);
        const auto handle_offset_b = cv::Vec3d(cb[0], s.marker_height_cm, cb[1]);

        const auto head_tvec_a = marker_tvecs[0] + handle_offset_a;
        const auto head_tvec_b = marker_tvecs[1] + handle_offset_b;

        const double da = fabs(ca[0]);
        const double db = fabs(cb[0]);

        if (da + db > 0)
            return (head_tvec_a * db + head_tvec_b * da) / (da + db);
        else
            return (head_tvec_a + head_tvec_b) / 2.0;
    }
}

/* Add the given detections to the marker model if the angle bin for pose_rvec has been visited fewer than max_visits times.
   The model is rebuilt so that later checks in the same frame (marker count, expected visible markers) see any new markers.
*/
bool PaperTracker::update_marker_model_at_pose(const cv::Vec3d &pose_rvec, const std::vector<marker_detection_info> &detections, int max_visits, double marker_size_cm) {
    cv::Matx33d R;
    cv::Rodrigues(pose_rvec, R);
    const auto euler = rotation_matrix_to_euler_zyx(R);

    if (marker_model.get_visit_count(euler[0], euler[1]) >= max_visits)
        return false;

    marker_model.update(detections);
    marker_model.add_visit(euler[0], euler[1]);
    marker_model.build(key_marker_id, marker_size_cm, camera_matrix, dist_coeffs);

    return true;
}

/* Update per-marker counts of consecutive frames in which each marker has been detected. These are used to
   compute the stability scores for the markers. Counting is keyed to frame_number so that processing the
   same camera frame more than once (ROI pass followed by full-frame pass) doesn't count it twice. A marker
   missing from one or more frames starts over from 1 the next time it is seen.
*/
void PaperTracker::update_marker_streaks(const std::vector<marker_detection_info> &detections) {
    for (const auto &detection : detections) {
        auto [it, inserted] = marker_streaks.try_emplace(detection.id, marker_streak{ frame_number, 1 });

        if (inserted)
            continue;

        auto &streak = it->second;

        if (streak.last_seen_frame == frame_number)
            continue; // already counted for this frame

        if (streak.last_seen_frame + 1 == frame_number)
            streak.count++;
        else
            streak.count = 1;

        streak.last_seen_frame = frame_number;
    }
}

/* Get a marker's stability score: the number of consecutive frames it has been detected in, including the
   current one, divided by PAPERTRACKER_MARKER_STABILITY_SCORE_FRAMES. A marker not detected in the current
   frame scores 0. The score starts at 1/PAPERTRACKER_MARKER_STABILITY_SCORE_FRAMES and is uncapped.
*/
double PaperTracker::get_marker_stability_score(int id) const {
    const auto it = marker_streaks.find(id);

    if (it == marker_streaks.end() || it->second.last_seen_frame != frame_number)
        return 0.0;

    return static_cast<double>(it->second.count) / PAPERTRACKER_MARKER_STABILITY_SCORE_FRAMES;
}

/* Determine if a marker's stability score is high enough for its detections to be used to update the marker model.
*/
bool PaperTracker::is_marker_stable(int id) const {
    return get_marker_stability_score(id) >= PAPERTRACKER_MARKER_STABILITY_SCORE_THRESHOLD;
}

/* Get the subset of solved detections whose markers are considered stable.
*/
const std::vector<marker_detection_info> &PaperTracker::get_stable_markers(const std::vector<marker_detection_info> &detections) {
    frame_data.stable_markers.clear();

    for (const auto &detection : detections)
        if (detection.solved && is_marker_stable(detection.id))
            frame_data.stable_markers.push_back(detection);

    return frame_data.stable_markers;
}

std::pair<cv::Vec3d, cv::Vec3d> PaperTracker::get_head_pose_from_marker_transform(cv::Vec3d &rvec_measured, cv::Vec3d &tvec_measured) {
    cv::Vec3d pose_rvec;
    cv::Vec3d pose_tvec;

    cv::Matx33d R_marker_set;
    cv::Rodrigues(rvec_measured, R_marker_set);

    cv::Matx33d R_key_marker;
    cv::Rodrigues(key_marker_local_rvec, R_key_marker);

    cv::Matx33d R_pose = R_marker_set * R_key_marker.t();
    cv::Rodrigues(R_pose, pose_rvec);

    pose_tvec = R_pose * -key_marker_local_tvec + tvec_measured;

    return {pose_rvec, pose_tvec};
}

/* Get a bounding box for a set of markers, its size increased by PAPERTRACKER_ROI_GROWTH_FACTOR.
*/
cv::Rect2f PaperTracker::get_marker_detected_region(const std::vector<marker_detection_info> &markers)
{
    if (markers.size() == 0 || markers[0].size() == 0)
        return cv::Rect2f(0, 0, 0, 0);

    cv::Point2f min = markers[0][0];
    cv::Point2f max = markers[0][0];

    for (const auto &marker : markers) {
        for (const auto &corner : marker) {
            min.x = std::min(corner.x, min.x);
            min.y = std::min(corner.y, min.y);
            max.x = std::max(corner.x, max.x);
            max.y = std::max(corner.y, max.y);
        }
    }

    const double w = max.x - min.x;
    const double h = max.y - min.y;

    min.x -= w * PAPERTRACKER_ROI_GROWTH_FACTOR / 2.0;
    min.y -= h * PAPERTRACKER_ROI_GROWTH_FACTOR / 2.0;
    max.x += w * PAPERTRACKER_ROI_GROWTH_FACTOR / 2.0;
    max.y += h * PAPERTRACKER_ROI_GROWTH_FACTOR / 2.0;

    return cv::Rect2f(min, max);
}

/* Determine if any markers have not been detected that should have been detected.
*/
bool PaperTracker::markers_disappeared(const std::vector<int> &expected, const std::vector<marker_detection_info> &detected) {
    for (const int expected_id : expected) {
        bool found = false;

        for (const auto marker : detected) {
            if (expected_id == marker.id) {
                found = true;
                break;
            }
        }

        if (!found)
            return true;
    }

    return false;
}

/* Draw reference points around the head from dimensions defined in settings.
*/
void PaperTracker::draw_head_indicator(cv::Mat &image)
{
    double head_radius = circumference_to_radius(s.head_circumference_cm);

    std::array<cv::Point3d, 8> points = {
        cv::Point3d(0, 0, 0),
        cv::Point3d(-head_radius, 0, -head_radius),
        cv::Point3d(head_radius, 0, -head_radius),
        cv::Point3d(-head_radius, 0, head_radius),
        cv::Point3d(head_radius, 0, head_radius),
        cv::Point3d(head_radius, 0, 0),
        cv::Point3d(0, head_radius, 0),
        cv::Point3d(0, 0, head_radius)
    };

    std::array<cv::Point2d, std::size(points)> image_points;

    cv::Mat points_mat(static_cast<int>(points.size()), 1, CV_64FC3, points.data());
    cv::Mat image_points_mat(static_cast<int>(image_points.size()), 1, CV_64FC2, image_points.data());

    cv::projectPoints(points_mat, head_rvec, head_tvec, camera_matrix, dist_coeffs, image_points_mat);

    const cv::Scalar brightness2(1, 1, 1);
    const cv::Scalar brightness1(0.5, 0.5, 0.5);

    const double line_thickness = std::max(image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 2.0);
    const double corner_radius = std::max(2 * image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 2.0);
    const double origin_radius = std::max(3 * image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 3.0);

    // back corners
    cv::circle(image, image_points[1], corner_radius, cv::Scalar(0, 255, 255) * brightness1, -1);
    cv::circle(image, image_points[2], corner_radius, cv::Scalar(0, 255, 255) * brightness1, -1);

    // line-clipping variables
    cv::Rect2d imageRect(0, 0, image.cols, image.rows);
    cv::Point2d p1;
    cv::Point2d p2;

    // x axis
    if (clip_line(imageRect, image_points[0], image_points[5], p1, p2)) {
        cv::line(image, p1, p2, cv::Scalar(255, 255, 255), line_thickness * 3);
        cv::line(image, p1, p2, cv::Scalar(0, 0, 255), line_thickness);
    }

    // y axis
    if (clip_line(imageRect, image_points[0], image_points[6], p1, p2)) {
        cv::line(image, p1, p2, cv::Scalar(255, 255, 255), line_thickness * 3);
        cv::line(image, p1, p2, cv::Scalar(0, 255, 0), line_thickness);
    }

    // z axis
    if (clip_line(imageRect, image_points[0], image_points[7], p1, p2)) {
        cv::line(image, p1, p2, cv::Scalar(255, 255, 255), line_thickness * 3);
        cv::line(image, p1, p2, cv::Scalar(255, 0, 0), line_thickness);
    }

    // origin
    cv::circle(image, image_points[0], origin_radius, cv::Scalar(0, 0, 255), -1);

    // front corners
    cv::circle(image, image_points[3], corner_radius, cv::Scalar(0, 255, 255) * brightness2, -1);
    cv::circle(image, image_points[4], corner_radius, cv::Scalar(0, 255, 255) * brightness2, -1);
}

/* Draw a border around a marker given its vertices in image space.
*/
void PaperTracker::draw_marker_border(cv::Mat &image, const std::array<cv::Point2f, 4> &image_points, int id, const cv::Scalar &border_color)
{
    const size_t vertex_count = image_points.size();

    cv::Point2f center(0, 0);

    const double line_thickness = std::max(image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 1.0);
    const double font_scale = line_thickness * 0.5;

    for (size_t v = 0; v < vertex_count; ++v) {
        cv::line(image, image_points[v], image_points[(v+1) % vertex_count], border_color, line_thickness);
        center += image_points[v];
    }

    center /= (double)image_points.size();

    std::stringstream ss;
    ss << id;

    const auto text_size = cv::getTextSize(ss.str(), cv::FONT_HERSHEY_SIMPLEX, font_scale, line_thickness, nullptr);

    cv::putText(image, ss.str(), center - cv::Point2f(text_size.width / 2.0, 0), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255) - border_color, line_thickness, cv::LINE_AA);
}

/* Update FPS average.
*/
void PaperTracker::update_fps()
{
    const double dt = fps_timer.elapsed_seconds();
    fps_timer.start();
    const double alpha = dt/(dt + 0.25);

    if (dt > 1e-3)
    {
        fps *= 1 - alpha;
        fps += alpha * (1./dt + .8);
    }
}

/* Start tracking.
*/
module_status PaperTracker::start_tracker(QFrame *videoframe)
{
    static_settings.frame_width = s.frame_width;
    static_settings.frame_height = s.frame_height;
    static_settings.fps = s.fps;
    static_settings.use_mjpeg = s.use_mjpeg;
    static_settings.camera_name = s.camera_name;
    static_settings.aruco_marker_size_mm = s.aruco_marker_size_mm;

    current_dictionary = s.aruco_dictionary;

    switch (current_dictionary) {
        case PAPERTRACKER_DICT_ARUCO_MIP_36h12:
            detector.setMakerDetectorFunction(detect_aruco_mip_36h12);
            break;

        case PAPERTRACKER_DICT_APRILTAG_36h11:
            detector.setMakerDetectorFunction(detect_apriltag_36h11);
            break;

        case PAPERTRACKER_DICT_ARUCO_ORIGINAL:
            detector.setMakerDetectorFunction(aruco::FiducidalMarkers::detect);
            break;
    }

    videoframe->show();

    videoWidget = std::make_unique<cv_video_widget>(videoframe);
    layout = std::make_unique<QHBoxLayout>();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&*videoWidget);
    videoframe->setLayout(&*layout);
    videoWidget->show();

     if (!open_camera())
        return error("Could not open camera.");

    head_rvec[0] = -CV_PI;
    head_rvec[1] = 0;
    head_rvec[2] = 0;

    last_marker_height_cm = s.marker_height_cm;
    last_head_circumference_cm = s.head_circumference_cm;

    start();

    return status_ok();
}

/* Main thread / update loop.
*/
void PaperTracker::run() {
    dist_coeffs = std::vector<double>(5, 0);

    update_fps();

    last_detection_timer.start();

    started_ = true;
    
    /* Update loop.
    */
    while (!isInterruptionRequested()) {
        video::frame frame;
        bool new_frame;

        {
            QMutexLocker l(&camera_mtx);

            std::tie(frame, new_frame) = camera->get_frame();

            if (!new_frame) {
                l.unlock();
                portable::sleep(100);
                continue;
            }
        }

        /* Frames must have 1 (grayscale) or 3 (color) channels.
        */
        if (frame.channels != 1 && frame.channels != 3) {
            qDebug() << "PaperTracker: can't handle" << frame.channels << "color channels";
            return;
        }

        auto frame_mat_temp = cv::Mat(frame.height, frame.width, CV_8UC(frame.channels), (void*)frame.data, frame.stride);

        /* Apply zoom (as region of interest).
        */
        const double zoom = s.zoom / 100.0;
        const int zoomed_width = std::max(frame_mat_temp.size().width / zoom, 1.0);
        const int zoomed_height = std::max(frame_mat_temp.size().height / zoom, 1.0);

        cv::Rect frame_rect(
            (frame_mat_temp.size().width - zoomed_width) / 2,
            (frame_mat_temp.size().height - zoomed_height) / 2,
            zoomed_width,
            zoomed_height
        );
        cv::Mat frame_mat = frame_mat_temp(frame_rect);

        marker_model.set_image_point_offset(cv::Vec2d(frame_rect.x, frame_rect.y));

        /* Adjust FOV according to zoom level and set camera matrix.
        */
        const double fov_rad = s.fov * (CV_PI / 180.0);
        const double cropped_fov = 2.0 * atan(tan(fov_rad/2.0) / zoom);
        camera_matrix = build_camera_matrix(frame_mat.size().width, frame_mat.size().height, cropped_fov);

        /* Process the current frame.
        */
        frame_number++;

        bool detection_ok = process_frame(frame_mat, &last_roi) || process_frame(frame_mat);

        if (detection_ok) {
            no_detection_timeout -= last_detection_timer.elapsed_seconds() * PAPERTRACKER_NO_DETECTION_TIMEOUT_BACKOFF_C;
            no_detection_timeout = std::fmax(0, no_detection_timeout);
            last_detection_timer.start();
        } else {
            no_detection_timeout += last_detection_timer.elapsed_seconds();

            last_detection_timer.start();

            if (no_detection_timeout > PAPERTRACKER_NO_DETECTION_TIMEOUT) {
                no_detection_timeout = 0;

                /* Cycle threshold parameters when the key marker has not yet been established, or when the
                   contrast method is set to continuous auto.
                */
                if (!has_key_marker || s.contrast_method == PAPERTRACKER_CONTRAST_METHOD_CONTINUOUS_AUTO) {
                    QMutexLocker l(&settings_update_mtx);
                    cycle_threshold_params();
                }
            }
        }

        // frame_mat.setTo(cv::Scalar(0, 0, 0));

        /* Draw detected markers.
        */
        for (auto &marker : detected_markers) {
            if (marker_highlight_set.count(marker.id) > 0)
                draw_marker_border(frame_mat, marker, marker.id, cv::Scalar(0, 0, 255));
            else
                draw_marker_border(frame_mat, marker, marker.id, cv::Scalar(0, 0, 170));
        }

        /* Draw head indicator.
        */
        if (has_key_marker)
            draw_head_indicator(frame_mat);

        /* Update and render FPS indicator.
        */
        update_fps();

        std::stringstream ss;
        ss << frame.width << "x" << frame.height << " @ " << int(fps) << "Hz";

        const double line_thickness = std::max(frame_mat.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 1.0);
        const double font_scale = line_thickness * 0.5;

        const auto text_size = cv::getTextSize(ss.str(), cv::FONT_HERSHEY_SIMPLEX, font_scale, line_thickness, nullptr);

        cv::putText(frame_mat, ss.str(), cv::Point(line_thickness * 5, text_size.height + line_thickness * 5), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(0, 255, 0), line_thickness, cv::LINE_AA);

        /* Show preview.
        */
        if (frame_mat.rows > 0)
            videoWidget->update_image(frame_mat);
    }
}

bool PaperTracker::tracking_started() const
{
    return started_;
}

void PaperTracker::update_settings()
{
    QMutexLocker l(&settings_update_mtx);

    if (current_dictionary != s.aruco_dictionary) {
        switch (s.aruco_dictionary) {
            case PAPERTRACKER_DICT_ARUCO_MIP_36h12:
                detector.setMakerDetectorFunction(detect_aruco_mip_36h12);
                break;

            case PAPERTRACKER_DICT_APRILTAG_36h11:
                detector.setMakerDetectorFunction(detect_apriltag_36h11);
                break;

            case PAPERTRACKER_DICT_ARUCO_ORIGINAL:
                detector.setMakerDetectorFunction(aruco::FiducidalMarkers::detect);
                break;
        }

        current_dictionary = s.aruco_dictionary;
    }
}

bool PaperTracker::restart_required() const
{
    return
        static_settings.frame_width != s.frame_width ||
        static_settings.frame_height != s.frame_height ||
        static_settings.fps != s.fps ||
        static_settings.use_mjpeg != s.use_mjpeg ||
        static_settings.camera_name != s.camera_name ||
        static_settings.aruco_marker_size_mm != s.aruco_marker_size_mm;
}

/* Supply position and orientation data.
*/
void PaperTracker::data(double *data)
{
    QMutexLocker l(&data_mtx);

    cv::Matx33d Rx;
    cv::Vec3d rvecX = { CV_PI, 0, 0 };
    cv::Rodrigues(rvecX, Rx);

    cv::Matx33d R;
    cv::Rodrigues(head_rvec, R);
    auto euler = rotation_matrix_to_euler_zyx(R * Rx);

    const auto origin_delta = current_head_origin - starting_head_origin;

    data[TX] = head_tvec[0] - origin_delta[0];
    data[TY] = -head_tvec[1] + origin_delta[1];
    data[TZ] = head_tvec[2] - origin_delta[2];

    data[Roll] = euler[2] * (180.0 / CV_PI);
    data[Pitch] = -euler[0] * (180.0 / CV_PI);
    data[Yaw] = euler[1] * (180.0 / CV_PI);
}

/* Tracker constructor.
*/
PaperTracker::PaperTracker() :
    has_key_marker(false),
    key_marker_id(0),
    last_marker_height_cm(0),
    last_head_circumference_cm(0),
    visited_angles(2.0 * CV_PI / PAPERTRACKER_ANGLE_COVERAGE_PITCH_STEPS, 2.0 * CV_PI / PAPERTRACKER_ANGLE_COVERAGE_YAW_STEPS),
    last_roi(0, 0, std::numeric_limits<float>::max(), std::numeric_limits<float>::max()),
    last_bin(std::numeric_limits<int>::max(), std::numeric_limits<int>::max()),
    started_(false),
    use_fixed_threshold(false),
    adaptive_size_pos(0),
    consecutive_transformation_error_frames(0)
{
    opencv_init();
    set_threshold_params(use_fixed_threshold, adaptive_threshold_sizes[adaptive_size_pos]);
    current_dictionary = s.aruco_dictionary;
    marker_highlight_set.reserve(16);
    marker_model.reserve(16);
}

/* Tracker destructor.
*/
PaperTracker::~PaperTracker() {
    requestInterruption();
    wait();
    portable::sleep(1000);
}

/* Tracker declaration.
*/
OPENTRACK_DECLARE_TRACKER(PaperTracker, PaperTrackerDialog, PaperTrackerMetadata)
