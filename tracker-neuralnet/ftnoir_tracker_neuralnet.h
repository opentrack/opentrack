/* Copyright (c) 2021 Michael Welter <michael@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "options/options.hpp"
#include "api/plugin-api.hpp"
#include "cv/video-widget.hpp"
#include "cv/translation-calibrator.hpp"
#include "cv/numeric.hpp"
#include "compat/timer.hpp"
#include "video/camera.hpp"
#include "cv/affine.hpp"

#include <QObject>
#include <QThread>
#include <QMutex>
#include <QHBoxLayout>
#include <QDialog>
#include <QTimer>

#include <memory>
#include <cinttypes>
#include <array>

#include <onnxruntime_cxx_api.h>

#include <opencv2/core.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>

#include "ui_neuralnet-trackercontrols.h"

namespace neuralnet_tracker_ns
{


using namespace options;


enum fps_choices
{
    fps_default = 0,
    fps_30      = 1,
    fps_60      = 2,
    fps_MAX     = 3
};

struct resolution_tuple
{
    int width;
    int height;
};

static const std::array<resolution_tuple, 7> resolution_choices =
{{
    { 320, 240 },
    { 640, 480 },
    { 800, 600 },
    { 1024, 768 },
    { 1280, 720 },
    { 1920, 1080},
    { 0, 0 }
}};


struct Settings : opts {
    value<int> offset_fwd { b, "offset-fwd", 200 }, // Millimeters
               offset_up { b, "offset-up", 0 },
               offset_right { b, "offset-right", 0 };
    value<QString> camera_name { b, "camera-name", ""};
    value<int> fov { b, "field-of-view", 56 };
    value<fps_choices> force_fps { b, "force-fps", fps_default };
    value<bool> show_network_input { b, "show-network-input", false };
    value<double> roi_filter_alpha{ b, "roi-filter-alpha", 1. };
    value<double> roi_zoom{ b, "roi-zoom", 1. };
    value<bool> use_mjpeg { b, "use-mjpeg", false };
    value<int> num_threads { b, "num-threads", 1 };
    value<int> resolution { b, "force-resolution", 0 };
    Settings();
};


struct CamIntrinsics
{
    float focal_length_w;
    float focal_length_h;
    float fov_w;
    float fov_h;
};


class Localizer
{
    public:
        Localizer(Ort::MemoryInfo &allocator_info,
                    Ort::Session &&session);
        
        // Returns bounding wrt image coordinate of the input image
        // The preceeding float is the score for being a face normalized to [0,1].
        std::pair<float, cv::Rect2f> run(
            const cv::Mat &frame);

        double last_inference_time_millis() const;
    private:
        inline static constexpr int INPUT_IMG_WIDTH = 288;
        inline static constexpr int INPUT_IMG_HEIGHT = 224;
        Ort::Session session_{nullptr};
        // Inputs / outputs
        cv::Mat scaled_frame_{}, input_mat_{};
        Ort::Value input_val_{nullptr}, output_val_{nullptr};
        std::array<float, 5> results_;
        double last_inference_time_ = 0;
};


class PoseEstimator
{
    public:
        struct Face
        {
            std::array<float,4> rotation; // Quaternion, (w, x, y, z)
            cv::Rect2f box;
            cv::Point2f center;
            float size;
        };

        PoseEstimator(Ort::MemoryInfo &allocator_info,
                        Ort::Session &&session);
        /** Inference
        *
        * Coordinates are defined wrt. the image space of the input `frame`.
        * X goes right, Z (depth) into the image, Y points down (like pixel coordinates values increase from top to bottom)
        */
        std::optional<Face> run(const cv::Mat &frame, const cv::Rect &box);
        // Returns an image compatible with the 'frame' image for displaying.
        cv::Mat last_network_input() const;
        double last_inference_time_millis() const;
    private:
        // Operates on the private image data members
        int find_input_intensity_90_pct_quantile() const;

        int64_t model_version_ = 0;  // Queried meta data from the ONNX file
        Ort::Session session_{nullptr};  // ONNX's runtime context for running the model
        Ort::Allocator allocator_;   // Memory allocator for tensors
        // Inputs
        cv::Mat scaled_frame_{}, input_mat_{};  // Input. One is the original crop, the other is rescaled (?)
        std::vector<Ort::Value> input_val_;    // Tensors to put into the model
        std::vector<const char*> input_names_; // Refers to the names in the onnx model. 
        // Outputs
        cv::Vec<float, 3> output_coord_{};  // 2d Coordinate and head size output.
        cv::Vec<float, 4> output_quat_{};   //  Quaternion output
        cv::Vec<float, 4> output_box_{};    // Bounding box output
        std::vector<Ort::Value> output_val_; // Tensors to put the model outputs in.
        std::vector<const char*> output_names_; // Refers to the names in the onnx model. 
        size_t num_recurrent_states_ = 0;
        double last_inference_time_ = 0;
};


class Preview
{
public:
    void init(const cv_video_widget& widget);
    void copy_video_frame(const cv::Mat& frame);
    void draw_gizmos(
        const std::optional<PoseEstimator::Face> &face,
        const Affine& pose,
        const std::optional<cv::Rect2f>& last_roi,
        const std::optional<cv::Rect2f>& last_localizer_roi,
        const cv::Point2f& neckjoint_position);
    void overlay_netinput(const cv::Mat& netinput);
    void draw_fps(double fps, double last_inference_time);
    void copy_to_widget(cv_video_widget& widget);
private:
    // Transform from camera frame to preview
    cv::Rect2f transform(const cv::Rect2f& r) const;
    cv::Point2f transform(const cv::Point2f& p) const;
    float transform(float s) const;

    cv::Mat preview_image_;
    cv::Size preview_size_ = { 0, 0 };
    float scale_ = 1.f;  
    cv::Point2f offset_ = { 0.f, 0.f};
};


class NeuralNetTracker : protected virtual QThread, public ITracker
{
    Q_OBJECT
public:
    NeuralNetTracker();
    ~NeuralNetTracker() override;
    module_status start_tracker(QFrame* frame) override;
    void data(double *data) override;
    void run() override;
    Affine pose();
    std::tuple<cv::Size, double, double> stats() const;

    QMutex camera_mtx_;
    std::unique_ptr<video::impl::camera> camera_;

private:
    bool detect();
    bool open_camera();
    void set_intrinsics();
    cv::Mat prepare_input_image(const video::frame& frame);
    bool load_and_initialize_model();
    void draw_gizmos(
        const std::optional<PoseEstimator::Face> &face,
        const Affine& pose);
    void update_fps(double dt);
    Affine compute_pose(const PoseEstimator::Face &face) const;

    Settings settings_;
    std::optional<Localizer> localizer_;
    std::optional<PoseEstimator> poseestimator_;
    Ort::Env env_{nullptr};
    Ort::MemoryInfo allocator_info_{nullptr};

    CamIntrinsics intrinsics_{};
    cv::Mat grayscale_;
    std::array<cv::Mat,2> downsized_original_images_ = {}; // Image pyramid
    std::optional<cv::Rect2f> last_localizer_roi_;
    std::optional<cv::Rect2f> last_roi_;
    static constexpr float HEAD_SIZE_MM = 200.f;

    mutable QMutex stats_mtx_;
    double fps_ = 0;
    double inference_time_ = 0;
    cv::Size resolution_ = {};
    
    static constexpr double RC = .25;
    int num_threads_ = 1;
    bool is_visible_ = true;

    QMutex mtx_; // Protects the pose
    Affine pose_;

    Preview preview_;
    std::unique_ptr<cv_video_widget> video_widget_;
    std::unique_ptr<QHBoxLayout> layout_;
};


class NeuralNetDialog : public ITrackerDialog
{
    Q_OBJECT
public:
    NeuralNetDialog();
    void register_tracker(ITracker * x) override;
    void unregister_tracker() override;
private:
    void make_fps_combobox();
    void make_resolution_combobox();

    Ui::Form ui_;
    Settings settings_;
    // Calibration code mostly taken from point tracker
    QTimer calib_timer_;
    TranslationCalibrator trans_calib_;
    QMutex calibrator_mutex_;
    QTimer tracker_status_poll_timer_;
    NeuralNetTracker* tracker_ = nullptr;

private Q_SLOTS:
    void doOK();
    void doCancel();
    void camera_settings();
    void update_camera_settings_state(const QString& name);
    void startstop_trans_calib(bool start);
    void trans_calib_step();
    void status_poll();
};


class NeuralNetMetadata : public Metadata
{
    Q_OBJECT
    QString name() override { return QString("neuralnet tracker"); }
    QIcon icon() override { return QIcon(":/images/neuralnet.png"); }
};


} // neuralnet_tracker_ns

using neuralnet_tracker_ns::NeuralNetTracker;
using neuralnet_tracker_ns::NeuralNetDialog;
using neuralnet_tracker_ns::NeuralNetMetadata;
