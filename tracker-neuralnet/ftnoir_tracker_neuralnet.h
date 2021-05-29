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


struct settings : opts {
    value<int> offset_fwd { b, "offset-fwd", 200 }, // Millimeters
               offset_up { b, "offset-up", 0 },
               offset_right { b, "offset-right", 0 };
    value<QString> camera_name { b, "camera-name", ""};
    value<int> fov { b, "field-of-view", 56 };
    value<fps_choices> force_fps { b, "force-fps", fps_default };
    value<bool> show_network_input { b, "show-network-input", false };
    settings();
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
        inline static constexpr int input_img_width = 288;
        inline static constexpr int input_img_height = 224;
        Ort::Session session{nullptr};
        // Inputs / outputs
        cv::Mat scaled_frame{}, input_mat{};
        Ort::Value input_val{nullptr}, output_val{nullptr};
        std::array<float, 5> results;
        double last_inference_time = 0;
};


class PoseEstimator
{
    public:
        struct Face
        {
            std::array<float,4> rotation; // Quaternion, (w, x, y, z)
            // The following quantities are defined wrt the image space of the input
            cv::Rect2f box;
            cv::Point2f center;
            float size;
        };

        PoseEstimator(Ort::MemoryInfo &allocator_info,
                        Ort::Session &&session);
        // Inference
        std::optional<Face> run(const cv::Mat &frame, const cv::Rect &box);
        // Returns an image compatible with the 'frame' image for displaying.
        cv::Mat last_network_input() const;
        double last_inference_time_millis() const;
    private:
        // Operates on the private image data members
        int find_input_intensity_90_pct_quantile() const;
        inline static constexpr int input_img_width = 129;
        inline static constexpr int input_img_height = 129;
        Ort::Session session{nullptr};
        // Inputs
        cv::Mat scaled_frame{}, input_mat{};
        Ort::Value input_val{nullptr};
        // Outputs
        cv::Vec<float, 3> output_coord{};
        cv::Vec<float, 4> output_quat{};
        cv::Vec<float, 4> output_box{};
        Ort::Value output_val[3] = {
            Ort::Value{nullptr}, 
            Ort::Value{nullptr}, 
            Ort::Value{nullptr}};
        double last_inference_time = 0;
};


class neuralnet_tracker : protected virtual QThread, public ITracker
{
    Q_OBJECT
public:
    neuralnet_tracker();
    ~neuralnet_tracker() override;
    module_status start_tracker(QFrame* frame) override;
    void data(double *data) override;
    void run() override;
    Affine pose();

    QMutex camera_mtx;
    std::unique_ptr<video::impl::camera> camera;

private:
    bool detect();
    bool open_camera();
    void set_intrinsics();
    bool load_and_initialize_model();
    void draw_gizmos(
        cv::Mat frame,  
        const std::optional<PoseEstimator::Face> &face,
        const Affine& pose) const;
    void update_fps(double dt);

    Affine compute_pose(const PoseEstimator::Face &face) const;
    numeric_types::vec3 image_to_world(float x, float y, float size, float real_size) const;
    numeric_types::vec2 world_to_image(const numeric_types::vec3& p) const;

    settings s;
    std::optional<Localizer> localizer;
    std::optional<PoseEstimator> poseestimator;
    Ort::Env env{nullptr};
    Ort::MemoryInfo allocator_info{nullptr};

    CamIntrinsics intrinsics{};
    cv::Mat frame, grayscale;
    std::optional<cv::Rect2f> last_localizer_roi;
    std::optional<cv::Rect2f> last_roi;
    static constexpr float head_size_mm = 200.f;

    double fps = 0;
    double last_inference_time = 0;
    static constexpr double RC = .25;

    QMutex mtx; // Protects the pose
    Affine pose_;

    std::unique_ptr<cv_video_widget> videoWidget;
    std::unique_ptr<QHBoxLayout> layout;
};


class neuralnet_dialog : public ITrackerDialog
{
    Q_OBJECT
public:
    neuralnet_dialog();
    void register_tracker(ITracker * x) override;
    void unregister_tracker() override;
private:
    void make_fps_combobox();

    Ui::Form ui;
    settings s;
    
    // Calibration code mostly taken from point tracker
    QTimer calib_timer;
    TranslationCalibrator trans_calib;
    QMutex calibrator_mutex;

    neuralnet_tracker* tracker = nullptr;

private Q_SLOTS:
    void doOK();
    void doCancel();
    void camera_settings();
    void update_camera_settings_state(const QString& name);
    void startstop_trans_calib(bool start);
    void trans_calib_step();
};


class neuralnet_metadata : public Metadata
{
    Q_OBJECT
    QString name() override { return QString("neuralnet tracker"); }
    QIcon icon() override { return QIcon(":/images/neuralnet.png"); }
};


} // neuralnet_tracker_ns

using neuralnet_tracker_ns::neuralnet_tracker;
using neuralnet_tracker_ns::neuralnet_dialog;
using neuralnet_tracker_ns::neuralnet_metadata;