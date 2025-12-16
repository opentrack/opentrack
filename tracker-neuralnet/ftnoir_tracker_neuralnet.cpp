/* Copyright (c) 2021 Michael Welter <michael@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_neuralnet.h"
#include "deadzone_filter.h"
#include "opencv_contrib.h"

#include "compat/check-visible.hpp"
#include "compat/math-imports.hpp"
#include "compat/sleep.hpp"
#include "compat/timer.hpp"
#include "cv/init.hpp"

#include <omp.h>
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>
#include <opencv2/core/quaternion.hpp>

#ifdef _MSC_VER
#pragma warning(disable : 4702)
#endif

#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QMutexLocker>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <unordered_map>

// Some demo code for onnx
// https://github.com/microsoft/onnxruntime/blob/master/csharp/test/Microsoft.ML.OnnxRuntime.EndToEndTests.Capi/C_Api_Sample.cpp
// https://github.com/leimao/ONNX-Runtime-Inference/blob/main/src/inference.cpp

namespace neuralnet_tracker_ns
{

using namespace cvcontrib;

using f = float;
template <int n> using vec = cv::Vec<f, n>;
template <int y, int x> using mat = cv::Matx<f, y, x>;
using vec2 = vec<2>;
using vec3 = vec<3>;
using mat33 = mat<3, 3>;

#if _MSC_VER
std::wstring convert(const QString& s)
{
    return s.toStdWString();
}
#else
std::string convert(const QString& s)
{
    return s.toStdString();
}
#endif

QDir get_default_model_directory()
{
    return QDir(OPENTRACK_BASE_PATH + "/" OPENTRACK_LIBRARY_PATH "models");
}

int enum_to_fps(int value)
{
    int fps = 0;

    switch (value)
    {
    default:
        eval_once(qDebug() << "neuralnet tracker: invalid fps enum value");
        [[fallthrough]];
    case fps_default:
        fps = 0;
        break;
    case fps_30:
        fps = 30;
        break;
    case fps_60:
        fps = 60;
        break;
    case fps_75:
        fps = 75;
        break;
    case fps_125:
        fps = 125;
        break;
    case fps_200:
        fps = 200;
        break;
    case fps_50:
        fps = 50;
        break;
    case fps_100:
        fps = 100;
        break;
    case fps_120:
        fps = 120;
        break;
    case fps_300:
        fps = 300;
        break;
    case fps_250:
        fps = 250;
        break;
    case fps_90:
        fps = 90;
        break;
    }

    return fps;
}

template <class F> struct OnScopeExit
{
    explicit OnScopeExit(F&& f) : f_{ f } {}
    ~OnScopeExit() noexcept { f_(); }
    F f_;
};

CamIntrinsics make_intrinsics(const cv::Mat& img, const Settings& settings)
{
    const int w = img.cols, h = img.rows;
    const double diag_fov = settings.fov * M_PI / 180.;
    const double fov_w = 2. * atan(tan(diag_fov / 2.) / sqrt(1. + h / (double)w * h / (double)w));
    const double fov_h = 2. * atan(tan(diag_fov / 2.) / sqrt(1. + w / (double)h * w / (double)h));
    const double focal_length_w = 1. / tan(.5 * fov_w);
    const double focal_length_h = 1. / tan(.5 * fov_h);
    /*  a
      ______  <--- here is sensor area
      |    /
      |   /
    f |  /
      | /  2 x angle is the fov
      |/
        <--- here is the hole of the pinhole camera

      So, a / f = tan(fov / 2)
      => f = a/tan(fov/2)
      What is a?
      1 if we define f in terms of clip space where the image plane goes from -1 to 1. Because a is the half-width.
    */

    return { (float)focal_length_w, (float)focal_length_h, (float)fov_w, (float)fov_h };
}

cv::Rect make_crop_rect_multiple_of(const cv::Size& size, int multiple)
{
    const int new_w = (size.width / multiple) * multiple;
    const int new_h = (size.height / multiple) * multiple;
    return cv::Rect((size.width - new_w) / 2, (size.height - new_h) / 2, new_w, new_h);
}

template <class T> cv::Rect_<T> squarize(const cv::Rect_<T>& r)
{
    cv::Point_<T> c{ r.x + r.width / T(2), r.y + r.height / T(2) };
    const T sz = std::max(r.height, r.width);
    return { c.x - sz / T(2), c.y - sz / T(2), sz, sz };
}

template <class T> cv::Rect_<T> expand(const cv::Rect_<T>& r, T factor)
{
    // xnew = l+.5*w - w*f*0.5 = l + .5*(w - new_w)
    const cv::Size_<T> new_size = { r.width * factor, r.height * factor };
    const cv::Point_<T> new_tl = r.tl() + (as_point(r.size()) - as_point(new_size)) / T(2);
    return cv::Rect_<T>(new_tl, new_size);
}

template <class T> cv::Rect_<T> ewa_filter(const cv::Rect_<T>& last, const cv::Rect_<T>& current, T alpha)
{
    const auto last_center = T(0.5) * (last.tl() + last.br());
    const auto cur_center = T(0.5) * (current.tl() + current.br());
    const cv::Point_<T> new_size = as_point(last.size()) + alpha * (as_point(current.size()) - as_point(last.size()));
    const cv::Point_<T> new_center = last_center + alpha * (cur_center - last_center);
    return cv::Rect_<T>(new_center - T(0.5) * new_size, as_size(new_size));
}

cv::Vec3f image_to_world(float x, float y, float size, float reference_size_in_mm, const cv::Size2i& image_size, const CamIntrinsics& intrinsics)
{
    /*
    Compute the location the network outputs in 3d space.

       hhhhhh  <- head size (meters)
      \      | -----------------------
       \     |                         \
        \    |                          |
         \   |                          |- x (meters)
          ____ <- face.size / width     |
           \ |  |                       |
            \|  |- focal length        /
               ------------------------
            ------------------------------------------------>> z direction
        z/x = zi / f
        zi = image position
        z = world position
        f = focal length

        We can also do deltas:
        dz / x = dzi / f
        => x = dz / dzi * f
        which means we can compute x from the head size (dzi) if we assume some reference size (dz).
    */
    const float head_size_vertical = 2.f * size; // Size from the model is more like half the real vertical size of a human head.
    const float xpos = -(intrinsics.focal_length_w * image_size.width * 0.5f) / head_size_vertical * reference_size_in_mm;
    const float zpos = (x / image_size.width * 2.f - 1.f) * xpos / intrinsics.focal_length_w;
    const float ypos = (y / image_size.height * 2.f - 1.f) * xpos / intrinsics.focal_length_h;
    return { xpos, ypos, zpos };
}

vec2 world_to_image(const cv::Vec3f& pos, const cv::Size2i& image_size, const CamIntrinsics& intrinsics)
{
    const float xscr = pos[2] / pos[0] * intrinsics.focal_length_w;
    const float yscr = pos[1] / pos[0] * intrinsics.focal_length_h;
    const float x = (xscr + 1.) * 0.5f * image_size.width;
    const float y = (yscr + 1.) * 0.5f * image_size.height;
    return { x, y };
}

cv::Quatf image_to_world(cv::Quatf q)
{
    std::swap(q[1], q[3]);
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
    return q;
}

cv::Point2f normalize(const cv::Point2f& p, int h, int w)
{
    return { p.x / w * 2.f - 1.f, p.y / h * 2.f - 1.f };
}

cv::Quatf rotation_from_two_vectors(const vec3& a, const vec3& b)
{
    // |axis| = |a| * |b| * sin(alpha)
    const vec3 axis = a.cross(b);
    // dot = |a|*|b|*cos(alpha)
    const float dot = a.dot(b);
    const float len = cv::norm(axis);
    vec3 normed_axis = axis / len;
    float angle = std::atan2(len, dot);
    if (!(std::isfinite(normed_axis[0]) && std::isfinite(normed_axis[1]) && std::isfinite(normed_axis[2])))
    {
        angle = 0.f;
        normed_axis = vec3{ 1., 0., 0. };
    }
    return cv::Quatf::createFromAngleAxis(angle, normed_axis);
}

// Computes correction due to head being off screen center.
cv::Quatf compute_rotation_correction(const cv::Point3f& p)
{
    return rotation_from_two_vectors({ -1.f, 0.f, 0.f }, p);
}

// Intersection over union. A value between 0 and 1 which measures the match between the bounding boxes.
template <class T> T iou(const cv::Rect_<T>& a, const cv::Rect_<T>& b)
{
    auto i = a & b;
    return double{ i.area() } / (a.area() + b.area() - i.area());
}

class GuardedThreadCountSwitch
{
    int old_num_threads_cv_ = 1;
    int old_num_threads_omp_ = 1;

public:
    GuardedThreadCountSwitch(int num_threads)
    {
        old_num_threads_cv_ = cv::getNumThreads();
        old_num_threads_omp_ = omp_get_num_threads();
        omp_set_num_threads(num_threads);
        cv::setNumThreads(num_threads);
    }

    ~GuardedThreadCountSwitch()
    {
        omp_set_num_threads(old_num_threads_omp_);
        cv::setNumThreads(old_num_threads_cv_);
    }

    GuardedThreadCountSwitch(const GuardedThreadCountSwitch&) = delete;
    GuardedThreadCountSwitch& operator=(const GuardedThreadCountSwitch&) = delete;
};

bool NeuralNetTracker::detect()
{
    double inference_time = 0.;

    OnScopeExit update_inference_time{ [&]()
                                       {
                                           QMutexLocker lck{ &stats_mtx_ };
                                           inference_time_ = inference_time;
                                       } };

    // If there is no past ROI from the localizer or if the match of its output
    // with the current ROI is too poor we have to run it again. This causes a
    // latency spike of maybe an additional 50%. But it only occurs when the user
    // moves his head far enough - or when the tracking ist lost ...
    if (!last_localizer_roi_ || !last_roi_ || iou(*last_localizer_roi_, *last_roi_) < 0.25)
    {
        auto [p, rect] = localizer_->run(grayscale_);
        inference_time += localizer_->last_inference_time_millis();

        if (last_roi_ && iou(rect, *last_roi_) >= 0.25 && p > 0.5)
        {
            // The new ROI matches the result from tracking, so the user is
            // still there and to not disturb recurrent models, we only update
            // ...
            last_localizer_roi_ = rect;
        }
        else if (p > 0.5 && rect.height > 32 && rect.width > 32)
        {
            // Tracking probably got lost since the ROI's don't match, but the
            // localizer still finds a face, so we use the ROI from the localizer
            last_localizer_roi_ = rect;
            last_roi_ = rect;
        }
        else
        {
            // Tracking lost and no localization result. The user probably can't be seen.
            last_roi_.reset();
            last_localizer_roi_.reset();
        }
    }

    if (!last_roi_)
    {
        // Last iteration the tracker failed to generate a trustworthy
        // roi and the localizer also cannot find a face.
        draw_gizmos({}, {});
        return false;
    }

    auto face = poseestimator_->run(grayscale_, *last_roi_);
    inference_time += poseestimator_->last_inference_time_millis();

    if (!face)
    {
        last_roi_.reset();
        draw_gizmos({}, {});
        return false;
    }

    cv::Rect2f roi = expand(face->box, (float)settings_.roi_zoom);

    last_roi_ = ewa_filter(*last_roi_, roi, float(settings_.roi_filter_alpha));

    QuatPose pose = compute_filtered_pose(*face);
    last_pose_ = pose;

    Affine pose_affine = { pose.rot.toRotMat3x3(cv::QUAT_ASSUME_UNIT), pose.pos };

    {
        QMutexLocker lck(&mtx_);
        last_pose_affine_ = pose_affine;
    }

    draw_gizmos(*face, last_pose_affine_);

    return true;
}

void NeuralNetTracker::draw_gizmos(const std::optional<PoseEstimator::Face>& face, const Affine& pose)
{
    if (!is_visible_)
        return;

    preview_.draw_gizmos(face, last_roi_, last_localizer_roi_, world_to_image(pose.t, grayscale_.size(), intrinsics_));

    if (settings_.show_network_input)
    {
        cv::Mat netinput = poseestimator_->last_network_input();
        preview_.overlay_netinput(netinput);
    }
}

QuatPose NeuralNetTracker::transform_to_world_pose(const cv::Quatf& face_rotation, const cv::Point2f& face_xy, const float face_size) const
{
    const vec3 face_world_pos = image_to_world(face_xy.x, face_xy.y, face_size, HEAD_SIZE_MM, grayscale_.size(), intrinsics_);

    const cv::Quatf rot_correction = compute_rotation_correction(face_world_pos);

    cv::Quatf rot = rot_correction * image_to_world(face_rotation);

    // But this is in general not the location of the rotation joint in the neck.
    // So we need an extra offset. Which we determine by computing
    // z,y,z-pos = head_joint_loc + R_face * offset
    const vec3 local_offset = vec3{ static_cast<float>(settings_.offset_fwd), static_cast<float>(settings_.offset_up), static_cast<float>(settings_.offset_right) };
    const vec3 offset = rotate(rot, local_offset);
    const vec3 pos = face_world_pos + offset;

    return { rot, pos };
}

QuatPose NeuralNetTracker::compute_filtered_pose(const PoseEstimator::Face& face)
{
    if (fps_ > 0.001 && last_pose_ && poseestimator_->has_uncertainty() && settings_.internal_filter_enabled)
    {
        auto image2world = [this](const cv::Quatf& face_rotation, const cv::Point2f& face_xy, const float face_size)
        { return this->transform_to_world_pose(face_rotation, face_xy, face_size); };

        return apply_filter(face, *last_pose_, 1. / fps_, std::move(image2world),
                            FiltParams{ float(settings_.deadzone_hardness), float(settings_.deadzone_size) });
    }
    else
    {
        return transform_to_world_pose(face.rotation, face.center, face.size);
    }
}

NeuralNetTracker::NeuralNetTracker()
{
    opencv_init();
    neuralnet_tracker_tests::run();
}

NeuralNetTracker::~NeuralNetTracker()
{
    requestInterruption();
    wait();
    // fast start/stop causes breakage
    portable::sleep(1000);
}

module_status NeuralNetTracker::start_tracker(QFrame* videoframe)
{
    // Set internal thread count as reference for initialization. It is used in 
    // several places and settings_.num_threads could theoretically change due
    // to concurrent accesses from other threads.
    num_threads_ = settings_.num_threads;

    if (!load_and_initialize_model())
        return error("Couldn't initialize the model.");

    if (!open_camera())
        return error("Couldn't open the camera.");

    videoframe->show();
    video_widget_ = std::make_unique<cv_video_widget>(videoframe);
    layout_ = std::make_unique<QHBoxLayout>();
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(&*video_widget_);
    videoframe->setLayout(&*layout_);
    video_widget_->show();
    start();
    return status_ok();
}

bool NeuralNetTracker::load_and_initialize_model()
{
    const QString localizer_model_path_enc = OPENTRACK_BASE_PATH + "/" OPENTRACK_LIBRARY_PATH "/models/head-localizer.onnx";
    const QString poseestimator_model_path_enc = get_posenet_filename();

    maybe_load_onnxruntime_dynamically();

    try
    {
        env_ = Ort::Env{ OrtLoggingLevel::ORT_LOGGING_LEVEL_ERROR, "tracker-neuralnet" };

        auto opts = Ort::SessionOptions{};
        // In older versions of the ONNX-RT there is a warning which says to control number of threads via OpenMP.
        // However, recently, OpenMP support was removed. Then this setting should work.
        opts.SetIntraOpNumThreads(num_threads_);
        opts.SetInterOpNumThreads(1);
        allocator_info_ = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        localizer_.emplace(allocator_info_, Ort::Session{ env_, convert(localizer_model_path_enc).c_str(), opts });

        qDebug() << "Loading pose net " << poseestimator_model_path_enc;
        poseestimator_.emplace(allocator_info_, Ort::Session{ env_, convert(poseestimator_model_path_enc).c_str(), opts });
    }
    catch (const Ort::Exception& e)
    {
        qDebug() << "Failed to initialize the neural network models. ONNX error message: " << e.what();
        return false;
    }
    catch (const std::exception& e)
    {
        qDebug() << "Failed to initialize the neural network models. Error message: " << e.what();
        return false;
    }

    return true;
}

bool NeuralNetTracker::open_camera()
{
    int rint = std::clamp(*settings_.resolution, 0, (int)std::size(resolution_choices) - 1);
    resolution_tuple res = resolution_choices[rint];
    int fps = enum_to_fps(settings_.force_fps);

    QMutexLocker l(&camera_mtx_);

    camera_ = video::make_camera(settings_.camera_name);

    if (!camera_)
        return false;

    video::impl::camera::info args{};

    if (res.width)
    {
        args.width = res.width;
        args.height = res.height;
    }
    if (fps)
        args.fps = fps;

    args.use_mjpeg = settings_.use_mjpeg;

    if (!camera_->start(args))
    {
        qDebug() << "neuralnet tracker: can't open camera";
        return false;
    }

    return true;
}

void NeuralNetTracker::run()
{
    preview_.init(*video_widget_);

    GuardedThreadCountSwitch switch_num_threads_to(num_threads_);

    std::chrono::high_resolution_clock clk;

    while (!isInterruptionRequested())
    {
        is_visible_ = check_is_visible();
        auto t = clk.now();
        {
            QMutexLocker l(&camera_mtx_);

            auto [img, res] = camera_->get_frame();

            if (!res)
            {
                l.unlock();
                portable::sleep(100);
                continue;
            }

            {
                QMutexLocker lck{ &stats_mtx_ };
                resolution_ = { img.width, img.height };
            }

            auto color = prepare_input_image(img);

            if (is_visible_)
                preview_.copy_video_frame(color);

            switch (img.channels)
            {
            case 1:
                grayscale_.create(img.height, img.width, CV_8UC1);
                color.copyTo(grayscale_);
                break;
            case 3:
                cv::cvtColor(color, grayscale_, cv::COLOR_BGR2GRAY);
                break;
            default:
                qDebug() << "Can't handle" << img.channels << "color channels";
                return;
            }
        }

        intrinsics_ = make_intrinsics(grayscale_, settings_);

        detect();

        if (is_visible_)
            preview_.copy_to_widget(*video_widget_);

        update_fps(std::chrono::duration_cast<std::chrono::milliseconds>(clk.now() - t).count() * 1.e-3);
    }
}

cv::Mat NeuralNetTracker::prepare_input_image(const video::frame& frame)
{
    auto img = cv::Mat(frame.height, frame.width, CV_8UC(frame.channels), (void*)frame.data, frame.stride);

    // Crop if aspect ratio is not 4:3
    if (img.rows * 4 != img.cols * 3)
    {
        img = img(make_crop_rect_for_aspect(img.size(), 4, 3));
    }

    img = img(make_crop_rect_multiple_of(img.size(), 4));

    if (img.cols > 640)
    {
        cv::pyrDown(img, downsized_original_images_[0]);
        img = downsized_original_images_[0];
    }
    if (img.cols > 640)
    {
        cv::pyrDown(img, downsized_original_images_[1]);
        img = downsized_original_images_[1];
    }

    return img;
}

void NeuralNetTracker::update_fps(double dt)
{
    const double alpha = dt / (dt + RC);
    if (dt > 1e-6)
    {
        QMutexLocker lck{ &stats_mtx_ };
        fps_ *= 1 - alpha;
        fps_ += alpha * 1. / dt;
    }
}

void NeuralNetTracker::data(double* data)
{
    Affine tmp = [&]()
    {
        QMutexLocker lck(&mtx_);
        return last_pose_affine_;
    }();

    const auto& mx = tmp.R.col(0);
    const auto& my = tmp.R.col(1);
    const auto& mz = tmp.R.col(2);

    // For reference: https://en.wikipedia.org/wiki/Euler_angles. Section "Rotation matrix". The relevant matrix is
    // under "Tait-Bryan angles", row with "Y_alpha Z_beta X_gamma = ...".
    // Because for the NN tracker x is forward, and y is up. We can see that the x axis is independent of roll. Thus it
    // is relatively easy to figure out the yaw and pitch angles (alpha and beta).
    const float yaw = std::atan2(mx(2), mx(0));
    const float pitch = -std::atan2(-mx(1), std::sqrt(mx(2) * mx(2) + mx(0) * mx(0)));
    // For the roll angle we recognize that the matrix entries in the second row contain cos(pitch)*cos(roll), and
    // cos(pitch)*sin(roll). Using atan2 eliminates the common pitch factor and we obtain the roll angle.
    const float roll = std::atan2(-mz(1), my(1));
    {
        constexpr double rad2deg = 180 / M_PI;
        data[Yaw] = rad2deg * yaw;
        data[Pitch] = rad2deg * pitch;
        data[Roll] = -rad2deg * roll;

        // convert to cm
        data[TX] = -tmp.t[2] * 0.1;
        data[TY] = tmp.t[1] * 0.1;
        data[TZ] = -tmp.t[0] * 0.1;
    }
}

Affine NeuralNetTracker::pose()
{
    QMutexLocker lck(&mtx_);
    return last_pose_affine_;
}

std::tuple<cv::Size, double, double> NeuralNetTracker::stats() const
{
    QMutexLocker lck(&stats_mtx_);
    return { resolution_, fps_, inference_time_ };
}

QString NeuralNetTracker::get_posenet_filename() const
{
    QString filename = settings_.posenet_file;
    if (QFileInfo(filename).isRelative())
        filename = get_default_model_directory().absoluteFilePath(filename);
    return filename;
}

void NeuralNetDialog::make_fps_combobox()
{
    for (int k = 0; k < fps_MAX; k++)
    {
        const int hz = enum_to_fps(k);
        const QString name = (hz == 0) ? tr("Default") : QString::number(hz);
        ui_.cameraFPS->addItem(name, k);
    }
}

void NeuralNetDialog::make_resolution_combobox()
{
    int k = 0;
    for (const auto [w, h] : resolution_choices)
    {
        const QString s = (w == 0) ? tr("Default") : QString::number(w) + " x " + QString::number(h);
        ui_.resolution->addItem(s, k++);
    }
}

NeuralNetDialog::NeuralNetDialog() : trans_calib_(1, 2)
{
    ui_.setupUi(this);

    make_fps_combobox();
    make_resolution_combobox();

    for (const auto& str : video::camera_names())
        ui_.cameraName->addItem(str);

    tie_setting(settings_.camera_name, ui_.cameraName);
    tie_setting(settings_.fov, ui_.cameraFOV);
    tie_setting(settings_.offset_fwd, ui_.tx_spin);
    tie_setting(settings_.offset_up, ui_.ty_spin);
    tie_setting(settings_.offset_right, ui_.tz_spin);
    tie_setting(settings_.show_network_input, ui_.showNetworkInput);
    tie_setting(settings_.roi_filter_alpha, ui_.roiFilterAlpha);
    tie_setting(settings_.use_mjpeg, ui_.use_mjpeg);
    tie_setting(settings_.roi_zoom, ui_.roiZoom);
    tie_setting(settings_.num_threads, ui_.threadCount);
    tie_setting(settings_.resolution, ui_.resolution);
    tie_setting(settings_.force_fps, ui_.cameraFPS);
    tie_setting(settings_.posenet_file, ui_.posenetFileDisplay);
    tie_setting(settings_.internal_filter_enabled, ui_.internal_filter_enabled);

    connect(ui_.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui_.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui_.camera_settings, SIGNAL(clicked()), this, SLOT(camera_settings()));
    connect(ui_.posenetSelectButton, SIGNAL(clicked()), this, SLOT(onSelectPoseNetFile()));
    connect(&settings_.camera_name, value_::value_changed<QString>(), this, &NeuralNetDialog::update_camera_settings_state);

    update_camera_settings_state(settings_.camera_name);

    connect(&calib_timer_, &QTimer::timeout, this, &NeuralNetDialog::trans_calib_step);
    calib_timer_.setInterval(35);
    connect(ui_.tcalib_button, SIGNAL(toggled(bool)), this, SLOT(startstop_trans_calib(bool)));

    connect(&tracker_status_poll_timer_, &QTimer::timeout, this, &NeuralNetDialog::status_poll);
    tracker_status_poll_timer_.setInterval(250);
    tracker_status_poll_timer_.start();
}

void NeuralNetDialog::save()
{
    settings_.b->save();
}

void NeuralNetDialog::reload()
{
    settings_.b->reload();
}

void NeuralNetDialog::doOK()
{
    save();
    close();
}

void NeuralNetDialog::doCancel()
{
    close();
}

void NeuralNetDialog::camera_settings()
{
    if (tracker_)
    {
        QMutexLocker l(&tracker_->camera_mtx_);
        (void)tracker_->camera_->show_dialog();
    }
    else
        (void)video::show_dialog(settings_.camera_name);
}

void NeuralNetDialog::update_camera_settings_state(const QString& name)
{
    (void)name;
    ui_.camera_settings->setEnabled(true);
}

void NeuralNetDialog::register_tracker(ITracker* x)
{
    tracker_ = static_cast<NeuralNetTracker*>(x);
    ui_.tcalib_button->setEnabled(true);
}

void NeuralNetDialog::unregister_tracker()
{
    tracker_ = nullptr;
    ui_.tcalib_button->setEnabled(false);
}

bool NeuralNetDialog::embeddable() noexcept
{
    return true;
}

void NeuralNetDialog::set_buttons_visible(bool x)
{
    ui_.buttonBox->setVisible(x);
}

void NeuralNetDialog::status_poll()
{
    QString status;
    if (!tracker_)
    {
        status = tr("Tracker Offline");
    }
    else
    {
        auto [res, fps, inference_time] = tracker_->stats();
        status = tr("%1x%2 @ %3 FPS / Inference: %4 ms").arg(res.width).arg(res.height).arg(int(fps)).arg(inference_time, 0, 'f', 1);
    }
    ui_.resolution_display->setText(status);
}

void NeuralNetDialog::trans_calib_step()
{
    if (tracker_)
    {
        const Affine X_CM = [&]()
        {
            QMutexLocker l(&calibrator_mutex_);
            return tracker_->pose();
        }();
        trans_calib_.update(X_CM.R, X_CM.t);
        auto [_, nsamples] = trans_calib_.get_estimate();

        constexpr int min_yaw_samples = 15;
        constexpr int min_pitch_samples = 12;
        constexpr int min_samples = min_yaw_samples + min_pitch_samples;

        // Don't bother counting roll samples. Roll calibration is hard enough
        // that it's a hidden unsupported feature anyway.

        QString sample_feedback;
        if (nsamples[0] < min_yaw_samples)
            sample_feedback = tr("%1 yaw samples. Yaw more to %2 samples for stable calibration.").arg(nsamples[0]).arg(min_yaw_samples);
        else if (nsamples[1] < min_pitch_samples)
            sample_feedback = tr("%1 pitch samples. Pitch more to %2 samples for stable calibration.").arg(nsamples[1]).arg(min_pitch_samples);
        else
        {
            const int nsamples_total = nsamples[0] + nsamples[1];
            sample_feedback = tr("%1 samples. Over %2, good!").arg(nsamples_total).arg(min_samples);
        }
        ui_.sample_count_display->setText(sample_feedback);
    }
    else
        startstop_trans_calib(false);
}

void NeuralNetDialog::startstop_trans_calib(bool start)
{
    QMutexLocker l(&calibrator_mutex_);
    // FIXME: does not work ...
    if (start)
    {
        qDebug() << "pt: starting translation calibration";
        calib_timer_.start();
        trans_calib_.reset();
        ui_.sample_count_display->setText(QString());
        // Tracker must run with zero'ed offset for calibration.
        settings_.offset_fwd = 0;
        settings_.offset_up = 0;
        settings_.offset_right = 0;
    }
    else
    {
        calib_timer_.stop();
        qDebug() << "pt: stopping translation calibration";
        {
            auto [tmp, nsamples] = trans_calib_.get_estimate();
            settings_.offset_fwd = int(tmp[0]);
            settings_.offset_up = int(tmp[1]);
            settings_.offset_right = int(tmp[2]);
        }
    }
    ui_.tx_spin->setEnabled(!start);
    ui_.ty_spin->setEnabled(!start);
    ui_.tz_spin->setEnabled(!start);

    if (start)
        ui_.tcalib_button->setText(tr("Stop calibration"));
    else
        ui_.tcalib_button->setText(tr("Start calibration"));
}

void NeuralNetDialog::onSelectPoseNetFile()
{
    const auto root = get_default_model_directory();
    // Start with the current setting
    QString filename = settings_.posenet_file;
    // If the filename is relative then assume that the file is located under the
    // model directory. Under regular use this should always be the case.
    if (QFileInfo(filename).isRelative())
        filename = root.absoluteFilePath(filename);
    filename = QFileDialog::getOpenFileName(this, tr("Select Pose Net ONNX"), filename, tr("ONNX Files (*.onnx)"));
    // In case the user aborted.
    if (filename.isEmpty())
        return;
    // When a file under the model directory was selected we can get rid of the
    // directory prefix. This is more robust than storing absolute paths, e.g.
    // in case the user moves the opentrack install folder / reuses old settings.
    // When the file is not in the model directory, we have to use the absolute path,
    // which is also fine as developer feature.
    if (filename.startsWith(root.absolutePath()))
        filename = root.relativeFilePath(filename);
    settings_.posenet_file = filename;
}

Settings::Settings() : opts("neuralnet-tracker")
{
}

} // namespace neuralnet_tracker_ns

OPENTRACK_DECLARE_TRACKER(NeuralNetTracker, NeuralNetDialog, NeuralNetMetadata)
