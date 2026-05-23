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
#include <opencv2/calib3d.hpp>
#include <opencv2/imgproc.hpp>
#include "config.h"
#include <unordered_map>
#include <unordered_set>
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

    if (static_settings.camera_name == "")
        camera_name = video::camera_names()[0];

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
    frame_data.pose_rvecs.clear();
    frame_data.pose_tvecs.clear();

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

    if (!has_key_marker)
        detect_markers_optimal(image, frame_data.returned_markers);
    else
        detector.detect(image, frame_data.returned_markers, cv::Mat(), cv::Mat(), -1, false);

    const int min_marker_id = s.first_marker_id;
    const int max_marker_id = s.first_marker_id + s.number_of_markers - 1;

    detected_markers.clear();
    for (const auto &marker : frame_data.returned_markers) {
        if (marker.id < min_marker_id || marker.id > max_marker_id)
            continue;

        detected_markers.push_back(marker_detection_info(marker.id, marker));
    }

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

    /* If no markers detected and ROI is not null, signal need to reset ROI.
    */
    bool detection_failed;

    if (detected_markers.size() == 0 && roi != nullptr)
        return false;
    else
        detection_failed = false;

    last_roi = get_marker_detected_region(detected_markers);

    /* Caclulate poses for detected markers.
    */
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

            /* Exclude markers that are too ambiguous, too close to head-on, or too oblique.
            */
            detected_markers[i].z_angle = get_marker_z_angle(detected_markers[i].rvec);

            if (error_ratio > PAPERTRACKER_MARKER_EXCLUSION_AMBIGUITY_THRESHOLD ||
                detected_markers[i].z_angle < CV_PI / 180.0 * s.marker_min_angle ||
                detected_markers[i].z_angle > CV_PI / 180.0 * s.marker_max_angle)
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
            std::sort(detected_markers.begin(), detected_markers.end(), [this](const auto &a, const auto &b) {
                const bool head_has_a = head.has_handle(a.id);
                const bool head_has_b = head.has_handle(b.id);

                if (head_has_a && !head_has_b)
                    return true;
                else if (!head_has_a && head_has_b)
                    return false;

                const double sorting_angle_a = fabs(a.z_angle - CV_PI / 180.0 * s.marker_min_angle);
                const double sorting_angle_b = fabs(b.z_angle - CV_PI / 180.0 * s.marker_min_angle);

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
            for (const auto marker_index : key_markers) {
                starting_rvecs.push_back(detected_markers[marker_index].rvec);
                starting_tvecs.push_back(detected_markers[marker_index].tvec);
            }

            key_marker_id = detected_markers[key_markers[0]].id;

            auto head_orientation = cv::Vec3d(CV_PI, 0, 0);
            auto head_origin = get_approximate_head_origin(starting_rvecs, starting_tvecs);

            auto [rvec_local, tvec_local] = get_marker_local_transform(starting_rvecs[0], starting_tvecs[0], head_orientation, head_origin);

            head.set_handle_origin(tvec_local);
            head.set_handle(Marker(key_marker_id, MeanVector(rvec_local, MeanVector::VectorType::ROTATION), MeanVector(tvec_local, MeanVector::VectorType::POLAR)));

            starting_head_origin = head_origin;
            current_head_origin = head_origin;

            has_key_marker = true;
        }
    } else if (last_head_circumference_cm != s.head_circumference_cm || last_marker_height_cm != s.marker_height_cm) {
        // update marker origin, if necessary
        auto head_orientation = cv::Vec3d(CV_PI, 0, 0);
        auto head_origin = get_approximate_head_origin(starting_rvecs, starting_tvecs);

        auto [rvec_local, tvec_local] = get_marker_local_transform(starting_rvecs[0], starting_tvecs[0], head_orientation, head_origin);

        head.set_handle_origin(tvec_local);

        last_head_circumference_cm = s.head_circumference_cm;
        last_marker_height_cm = s.marker_height_cm;

        current_head_origin = head_origin;
    }
    
    if (has_key_marker && !frame_data.selected_markers.empty()) {
        /* Compute poses for known markers.
        */
        for (size_t i = 0; i < frame_data.selected_markers.size(); ++i) {
            const auto marker_index = frame_data.selected_markers[i];
            const int id = detected_markers[marker_index].id;

            if (head.has_handle(id)) {
                const auto sample_count = head.get_handle(id).rvec_local.get_max_sample_count();

                if (sample_count < PAPERTRACKER_MIN_VECTOR_SAMPLES && frame_data.selected_markers.size() != 1)
                    continue;

                if (detected_markers[marker_index].solved) {
                    auto [pose_rvec, pose_tvec] = head.get_pose_from_handle_transform(id, detected_markers[marker_index].rvec, detected_markers[marker_index].tvec);

                    frame_data.pose_rvecs.push_back({id, pose_rvec});
                    frame_data.pose_tvecs.push_back({id, pose_tvec});
                }
            }
        }

        /* Add/update handles.
        */
        if (frame_data.pose_rvecs.size() > 0) {
            /* Update head pose.
            */
            {
                QMutexLocker l(&data_mtx);

                frame_data.temp_vecs.clear();
                for (const auto& [pose_marker_id, pose_rvec] : frame_data.pose_rvecs)
                    frame_data.temp_vecs.push_back(pose_rvec);

                head.rvec = average_rotation(frame_data.temp_vecs);

                frame_data.temp_vecs.clear();
                for (const auto& [pose_marker_id, pose_tvec] : frame_data.pose_tvecs)
                    frame_data.temp_vecs.push_back(pose_tvec);

                head.tvec = average_translation(frame_data.temp_vecs);
            }

            /* Compute local transforms for each marker, adding or updating handles as needed.
            */
            for (size_t i = 0; i < frame_data.selected_markers.size(); ++i) {
                const auto marker_index = frame_data.selected_markers[i];
                const int id = detected_markers[marker_index].id;

                if (detected_markers[marker_index].solved) {
                    if (!head.has_handle(id)) {
                        auto [rvec_local, tvec_local] = get_marker_local_transform(detected_markers[marker_index].rvec, detected_markers[marker_index].tvec, head.rvec, head.tvec);
                        head.set_handle(Marker(id, MeanVector(rvec_local, MeanVector::VectorType::ROTATION), MeanVector(tvec_local, MeanVector::VectorType::POLAR)));
                    } else if (frame_data.pose_rvecs.size() > 1 && id != key_marker_id) {
                        auto &handle = head.get_handle(id);

                        if (!handle.rvec_local.outliers_removed() && handle.rvec_local.sample_count() == handle.rvec_local.get_max_sample_count())
                            handle.rvec_local.remove_outliers();

                        if (!handle.tvec_local.outliers_removed() && handle.tvec_local.sample_count() == handle.tvec_local.get_max_sample_count())
                            handle.tvec_local.remove_outliers();

                        if (handle.rvec_local.sample_count() < handle.rvec_local.get_max_sample_count()) {
                            frame_data.temp_vecs.clear();
                            for (const auto& [pose_marker_id, pose_rvec] : frame_data.pose_rvecs)
                                if (pose_marker_id != id)
                                    frame_data.temp_vecs.push_back(pose_rvec);

                            const auto pose_rvec = average_rotation(frame_data.temp_vecs);

                            frame_data.temp_vecs.clear();
                            for (const auto& [pose_marker_id, pose_tvec] : frame_data.pose_tvecs)
                                if (pose_marker_id != id)
                                    frame_data.temp_vecs.push_back(pose_tvec);

                            const auto pose_tvec = average_translation(frame_data.temp_vecs);

                            auto [rvec_local, tvec_local] = get_marker_local_transform(detected_markers[marker_index].rvec, detected_markers[marker_index].tvec, pose_rvec, pose_tvec);

                            head.update_handle(handle.id, rvec_local, tvec_local);
                        }
                    }
                }
            }
        }

        /* Check detected markers against expectations and signal need to reset ROI if these don't match.
        */
        cv::Matx33d R;
        cv::Rodrigues(head.rvec, R);
        const auto euler = rotation_matrix_to_euler_zyx(R);

        const auto bin = visited_angles.get_bin(euler[0], euler[1]);

        if (bin != last_bin && visited_angles.get_visit_count(last_bin) < PAPERTRACKER_ANGLE_COVERAGE_VISIT_THRESHOLD)
            visited_angles.clear_visits(last_bin);

        if (roi == nullptr && visited_angles.get_visit_count(bin) < PAPERTRACKER_ANGLE_COVERAGE_VISIT_THRESHOLD)
            visited_angles.add_visit(bin);

        if (head.num_handles() < s.number_of_markers && visited_angles.get_visit_count(bin) < PAPERTRACKER_ANGLE_COVERAGE_VISIT_THRESHOLD) {
            detection_failed = true;
        } else {
            auto expected_ids = head.get_expected_visible_ids(CV_PI / 180.0 * s.marker_max_angle);

            if (markers_disappeared(expected_ids, detected_markers))
                detection_failed = true;
        }
    }

    return !detection_failed;
}

static const int adaptive_threshold_sizes[] =
{
    5,
    7,
    9,
    11
};

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

    set_threshold_params();

    qDebug() << "aruco: switched thresholding params"
             << "fixed:" << use_fixed_threshold
             << "size:" << adaptive_threshold_sizes[adaptive_size_pos];
}

/* Update detector thresholding parameters. Adapted from ftnoir_tracker_aruco.cpp.
*/
void PaperTracker::set_threshold_params()
{
    detector.setDesiredSpeed(3);

    if (use_fixed_threshold)
        detector._thresMethod = aruco::MarkerDetector::FIXED_THRES;
    else
        detector._thresMethod = aruco::MarkerDetector::ADPT_THRES;

    detector.setThresholdParams(adaptive_threshold_sizes[adaptive_size_pos], PAPERTRACKER_ADAPTIVE_THRESHOLD_C);
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
        set_threshold_params();

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
        set_threshold_params();

        detector.detect(image, frame_data.temp_markers, cv::Mat(), cv::Mat(), -1, false);

        if (frame_data.temp_markers.size() > best_markers.size()) {
            best_markers.assign(frame_data.temp_markers.begin(), frame_data.temp_markers.end());
            best_threshold_fixed = false;
            best_adaptive_size_pos = adaptive_size_pos;
        }
    }

    use_fixed_threshold = best_threshold_fixed;
    adaptive_size_pos = best_adaptive_size_pos;
    set_threshold_params();
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

    cv::projectPoints(points_mat, head.rvec, head.tvec, camera_matrix, dist_coeffs, image_points_mat);

    const cv::Scalar brightness2(1, 1, 1);
    const cv::Scalar brightness1(0.5, 0.5, 0.5);

    const double line_thickness = std::max(image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 2.0);
    const double corner_radius = std::max(2 * image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 2.0);
    const double origin_radius = std::max(3 * image.cols * PAPERTRACKER_LINE_THICKNESS_FRAME_SCALING_FACTOR, 3.0);

    // back corners
    cv::circle(image, image_points[1], corner_radius, cv::Scalar(0, 255, 255) * brightness1, -1);
    cv::circle(image, image_points[2], corner_radius, cv::Scalar(0, 255, 255) * brightness1, -1);

    // x axis
    cv::line(image, image_points[0], image_points[5], cv::Scalar(255, 255, 255), line_thickness * 3);
    cv::line(image, image_points[0], image_points[5], cv::Scalar(0, 0, 255), line_thickness);

    // y axis
    cv::line(image, image_points[0], image_points[6], cv::Scalar(255, 255, 255), line_thickness * 3);
    cv::line(image, image_points[0], image_points[6], cv::Scalar(0, 255, 0), line_thickness);

    // z axis
    cv::line(image, image_points[0], image_points[7], cv::Scalar(255, 255, 255), line_thickness * 3);
    cv::line(image, image_points[0], image_points[7], cv::Scalar(255, 0, 0), line_thickness);

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

    head.rvec[0] = -CV_PI;
    head.rvec[1] = 0;
    head.rvec[2] = 0;

    last_marker_height_cm = s.marker_height_cm;
    last_head_circumference_cm = s.head_circumference_cm;

    start();

    return status_ok();
}

/* Main thread / update loop.
*/
void PaperTracker::run() {
    dist_coeffs = std::vector<double>(5, 0);

    if (!open_camera())
        return;

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

        cv::Mat frame_mat = frame_mat_temp(cv::Rect((frame_mat_temp.size().width - zoomed_width)/2, (frame_mat_temp.size().height - zoomed_height)/2, zoomed_width, zoomed_height));

        /* Adjust FOV according to zoom level and set camera matrix.
        */
        const double fov_rad = s.fov * (CV_PI / 180.0);
        const double cropped_fov = 2.0 * atan(tan(fov_rad/2.0) / zoom);
        camera_matrix = build_camera_matrix(frame_mat.size().width, frame_mat.size().height, cropped_fov);

        /* Process the current frame.
        */
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
                cycle_threshold_params();
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
        if (head.num_handles() > 0)
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
    cv::Rodrigues(head.rvec, R);
    auto euler = rotation_matrix_to_euler_zyx(R * Rx);

    const auto origin_delta = current_head_origin - starting_head_origin;

    data[TX] = head.tvec[0] - origin_delta[0];
    data[TY] = -head.tvec[1] + origin_delta[1];
    data[TZ] = head.tvec[2] - origin_delta[2];

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
    adaptive_size_pos(0)
{
    opencv_init();
    set_threshold_params();
    current_dictionary = s.aruco_dictionary;
    marker_highlight_set.reserve(16);
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
