/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include <QThread>
#include <QHBoxLayout>
#include <QMutex>
#include <unordered_map>
#include "api/plugin-api.hpp"
#include "cv/video-widget.hpp"
#include "video/camera.hpp"
#include "compat/timer.hpp"
#include "aruco/markerdetector.h"
#include "aruco/arucofidmarkers.h"
#include "papertracker-dialog.h"
#include "anglecoveragetracker.h"
#include "marker-model.h"
#include "marker-detection-info.h"

using papertracker::marker_detection_info;

class PaperTrackerDialog;

class PaperTracker : protected virtual QThread, public ITracker
{
public:
    PaperTracker();
    ~PaperTracker() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
    void run() override;
    bool tracking_started() const;
    void update_settings();
    bool restart_required() const;

private:
    struct frame_data_ {
        /* List of markers returned by ArUco detector.
        */
        std::vector<aruco::Marker> returned_markers;

        /* Temporary list of markers.
        */
        std::vector<aruco::Marker> temp_markers;

        /* Temporary list of rotation vectors.
        */
        std::vector<cv::Vec3d> temp_rvecs;

        /* Temporary list of translation vectors.
        */
        std::vector<cv::Vec3d> temp_tvecs;

        /* Temporary list of marker weights.
        */
        std::vector<double> temp_weights;

        /* Set of markers that are candidates for exclusion.
        */
        std::unordered_set<int> excluded_markers;

        /* Markers to use for pose estimation.
        */
        std::vector<size_t> selected_markers;

        /* Lists of values returned by solvePnPGeneric.
        */
        std::vector<cv::Vec3d> pnp_rvecs;
        std::vector<cv::Vec3d> pnp_tvecs;
        std::vector<double> pnp_reprojection_errors;

        /* Detected markers whose detections have been stable long enough to be used for model updates.
        */
        std::vector<marker_detection_info> stable_markers;

        frame_data_() {
            const size_t reserve_count = 16;

            returned_markers.reserve(reserve_count);
            temp_markers.reserve(reserve_count);

            temp_rvecs.reserve(reserve_count);
            temp_tvecs.reserve(reserve_count);
            temp_weights.reserve(reserve_count);

            excluded_markers.reserve(reserve_count);
            selected_markers.reserve(reserve_count);

            pnp_rvecs.reserve(2);
            pnp_tvecs.reserve(2);
            pnp_reprojection_errors.reserve(2);

            stable_markers.reserve(reserve_count);
        }
    } frame_data;

    cv::Vec3d head_rvec;
    cv::Vec3d head_tvec;
    cv::Vec3d key_marker_local_rvec;
    cv::Vec3d key_marker_local_tvec;
    aruco::MarkerDetector detector;
    papertracker_dictionary current_dictionary;
    std::unique_ptr<video::impl::camera> camera;
    cv::Matx33d camera_matrix;
    std::vector<double> dist_coeffs;
    bool has_key_marker;
    int key_marker_id;
    std::vector<marker_detection_info> detected_markers;
    std::vector<cv::Vec3d> starting_rvecs;
    std::vector<cv::Vec3d> starting_tvecs;
    cv::Vec3d starting_head_origin;
    cv::Vec3d current_head_origin;
    double last_marker_height_cm;
    double last_head_circumference_cm;
    std::unordered_set<int> marker_highlight_set;
    papertracker::AngleCoverageTracker visited_angles;
    cv::Rect2i last_roi;
    papertracker::AngleCoverageBin last_bin;
    papertracker::MarkerModel marker_model;
    papertracker_settings s;
    papertracker_static_settings static_settings;
    std::unique_ptr<cv_video_widget> videoWidget;
    std::unique_ptr<QHBoxLayout> layout;
    Timer fps_timer;
    Timer last_detection_timer;
    double fps = 0;
    double no_detection_timeout = 0;
    QMutex camera_mtx;
    QMutex data_mtx;
    QMutex settings_update_mtx;
    bool started_;
    bool use_fixed_threshold;
    unsigned int adaptive_size_pos;
    int consecutive_transformation_error_frames;

    struct marker_streak {
        uint64_t last_seen_frame;
        uint64_t count;
    };

    std::unordered_map<int, marker_streak> marker_streaks;
    uint64_t frame_number = 0;

    bool open_camera();
    bool process_frame(cv::Mat& frame, const cv::Rect2i *roi = nullptr);
    void cycle_threshold_params();
    void set_threshold_params(bool fixed_threshold, int adaptive_size);
    void apply_contrast_method();
    void detect_markers_optimal(const cv::Mat& image, std::vector<aruco::Marker> &best_markers);
    cv::Matx33d build_camera_matrix(int image_width, int image_height, double diagonal_fov);
    std::vector<size_t> get_key_markers(const std::vector<marker_detection_info> &detection_info);
    cv::Vec3d get_approximate_head_origin(const std::vector<cv::Vec3d> &marker_rvecs, const std::vector<cv::Vec3d> &marker_tvecs);
    std::pair<cv::Vec3d, cv::Vec3d> get_head_pose_from_marker_transform(cv::Vec3d &rvec_measured, cv::Vec3d &tvec_measured);
    void update_marker_streaks(const std::vector<marker_detection_info> &detections);
    double get_marker_stability_score(int id) const;
    bool is_marker_stable(int id) const;
    const std::vector<marker_detection_info> &get_stable_markers(const std::vector<marker_detection_info> &detections);
    bool update_marker_model_at_pose(const cv::Vec3d &pose_rvec, const std::vector<marker_detection_info> &detections, int max_visits, double marker_size_cm);
    cv::Rect2f get_marker_detected_region(const std::vector<marker_detection_info> &markers);
    bool markers_disappeared(const std::vector<int> &expected, const std::vector<marker_detection_info> &detected);
    void draw_head_indicator(cv::Mat &image);
    void draw_marker_border(cv::Mat &image, const std::array<cv::Point2f, 4> &image_points, int id, const cv::Scalar &marker_border = cv::Scalar(0, 0, 255));
    void update_fps();

    friend class PaperTrackerDialog;
};

class PaperTrackerMetadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("PaperTracker"); }
    QIcon icon() override { return QIcon(":/images/papertracker.png"); }
};

