/* Copyright (c) 2021 Michael Welter <michael@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_neuralnet.h"
#include "compat/sleep.hpp"
#include "compat/math-imports.hpp"
#include "cv/init.hpp"
#include <opencv2/core.hpp>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/types.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/imgcodecs.hpp>
#include "compat/timer.hpp"
#include "compat/check-visible.hpp"
#include <omp.h>

#ifdef _MSC_VER
#   pragma warning(disable : 4702)
#endif

#include <QMutexLocker>
#include <QDebug>
#include <QFile>

#include <cstdio>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <string>
#include <stdexcept>


// Some demo code for onnx
// https://github.com/microsoft/onnxruntime/blob/master/csharp/test/Microsoft.ML.OnnxRuntime.EndToEndTests.Capi/C_Api_Sample.cpp
// https://github.com/leimao/ONNX-Runtime-Inference/blob/main/src/inference.cpp

namespace neuralnet_tracker_ns
{


using numeric_types::vec3;
using numeric_types::vec2;
using numeric_types::mat33;
using quat = std::array<numeric_types::f,4>;


#if _MSC_VER
std::wstring convert(const QString &s) { return s.toStdWString(); }
#else
std::string convert(const QString &s) { return s.toStdString(); }
#endif


template<class F>
struct OnScopeExit
{
    explicit OnScopeExit(F&& f) : f_{ f } {}
    ~OnScopeExit() noexcept
    {
        f_();
    }
    F f_;
};


float sigmoid(float x)
{
    return 1.f/(1.f + std::exp(-x));
}


cv::Rect make_crop_rect_for_aspect(const cv::Size &size, int aspect_w, int aspect_h)
{
    auto [w, h] = size;
    if ( w*aspect_h > aspect_w*h )
    {
        // Image is too wide
        const int new_w = (aspect_w*h)/aspect_h;
        return cv::Rect((w - new_w)/2, 0, new_w, h);
    }
    else
    {
        const int new_h = (aspect_h*w)/aspect_w;
        return cv::Rect(0, (h - new_h)/2, w, new_h);
    }
}

cv::Rect make_crop_rect_multiple_of(const cv::Size &size, int multiple)
{
    const int new_w = (size.width / multiple) * multiple;
    const int new_h = (size.height / multiple) * multiple;
    return cv::Rect(
        (size.width-new_w)/2,
        (size.height-new_h)/2,
        new_w,
        new_h
    );
}

template<class T>
cv::Rect_<T> squarize(const cv::Rect_<T> &r)
{
    cv::Point_<T> c{r.x + r.width/T(2), r.y + r.height/T(2)};
    const T sz = std::max(r.height, r.width);
    return {c.x - sz/T(2), c.y - sz/T(2), sz, sz};
}


template<class T>
cv::Point_<T> as_point(const cv::Size_<T>& s)
{
    return { s.width, s.height };
}


template<class T>
cv::Size_<T> as_size(const cv::Point_<T>& p)
{
    return { p.x, p.y };
}


template<class T>
cv::Rect_<T> expand(const cv::Rect_<T>& r, T factor)
{
    // xnew = l+.5*w - w*f*0.5 = l + .5*(w - new_w)
    const cv::Size_<T> new_size = { r.width * factor, r.height * factor };
    const cv::Point_<T> new_tl = r.tl() + (as_point(r.size()) - as_point(new_size)) / T(2);
    return cv::Rect_<T>(new_tl, new_size);
}


template<class T>
cv::Rect_<T> ewa_filter(const cv::Rect_<T>& last, const cv::Rect_<T>& current, T alpha)
{
    const auto last_center = T(0.5) * (last.tl() + last.br());
    const auto cur_center = T(0.5) * (current.tl() + current.br());
    const cv::Point_<T> new_size = as_point(last.size()) + alpha * (as_point(current.size()) - as_point(last.size()));
    const cv::Point_<T> new_center = last_center + alpha * (cur_center - last_center);
    return cv::Rect_<T>(new_center - T(0.5) * new_size, as_size(new_size));
}


cv::Rect2f unnormalize(const cv::Rect2f &r, int h, int w)
{
    auto unnorm = [](float x) -> float { return 0.5*(x+1); };
    auto tl = r.tl();
    auto br = r.br();
    auto x0 = unnorm(tl.x)*w;
    auto y0 = unnorm(tl.y)*h;
    auto x1 = unnorm(br.x)*w;
    auto y1 = unnorm(br.y)*h;
    return {
        x0, y0, x1-x0, y1-y0
    };
}

cv::Point2f normalize(const cv::Point2f &p, int h, int w)
{
    return {
        p.x/w*2.f-1.f,
        p.y/h*2.f-1.f
    };
}


mat33 rotation_from_two_vectors(const vec3 &a, const vec3 &b)
{
    vec3 axis = a.cross(b);
    const float len_a = cv::norm(a);
    const float len_b = cv::norm(b);
    const float len_axis = cv::norm(axis);
    const float sin_angle = std::clamp(len_axis / (len_a * len_b), -1.f, 1.f);
    const float angle = std::asin(sin_angle);
    axis *= angle/(1.e-12 + len_axis);
    mat33 out;
    cv::Rodrigues(axis, out);
    return out;
}


// Computes correction due to head being off screen center.
// x, y: In screen space, i.e. in [-1,1]
// focal_length_x: In screen space
mat33 compute_rotation_correction(const cv::Point2f &p, float focal_length_x)
{
    return rotation_from_two_vectors(
        {1.f,0.f,0.f}, 
        {focal_length_x, p.y, p.x});
}


mat33 quaternion_to_mat33(const std::array<float,4> quat)
{
    mat33 m;
    const float w = quat[0];
    const float i = quat[1];
    const float j = quat[2];
    const float k = quat[3];
    m(0,0) = 1.f - 2.f*(j*j + k*k);
    m(1,0) =       2.f*(i*j + k*w);
    m(2,0) =       2.f*(i*k - j*w);
    m(0,1) =       2.f*(i*j - k*w);
    m(1,1) = 1.f - 2.f*(i*i + k*k);
    m(2,1) =       2.f*(j*k + i*w);
    m(0,2) =       2.f*(i*k + j*w);
    m(1,2) =       2.f*(j*k - i*w);
    m(2,2) = 1.f - 2.f*(i*i + j*j);
    return m;
}


vec3 rotate_vec(const quat& q, const vec3& p)
{
    const float qw = q[0];
    const float qi = q[1];
    const float qj = q[2];
    const float qk = q[3];
    const float pi = p[0];
    const float pj = p[1];
    const float pk = p[2];
    const quat tmp{
                        - qi*pi - qj*pj - qk*pk,
        qw*pi                   + qj*pk - qk*pj,
        qw*pj - qi*pk                   + qk*pi,
        qw*pk + qi*pj - qj*pi                  
    };
    const vec3 out {
        -tmp[0]*qi + tmp[1]*qw - tmp[2]*qk + tmp[3]*qj,
        -tmp[0]*qj + tmp[1]*qk + tmp[2]*qw - tmp[3]*qi,
        -tmp[0]*qk - tmp[1]*qj + tmp[2]*qi + tmp[3]*qw
    };
    return out;
}


vec3 image_to_world(float x, float y, float size, float reference_size_in_mm, const cv::Size2i& image_size, const CamIntrinsics& intrinsics)
{
    // Compute the location the network outputs in 3d space.
    const float xpos = -(intrinsics.focal_length_w * image_size.width * 0.5f) / size * reference_size_in_mm;
    const float zpos = (x / image_size.width * 2.f - 1.f) * xpos / intrinsics.focal_length_w;
    const float ypos = (y / image_size.height * 2.f - 1.f) * xpos / intrinsics.focal_length_h;
    return {xpos, ypos, zpos};
}


vec2 world_to_image(const vec3& pos, const cv::Size2i& image_size, const CamIntrinsics& intrinsics)
{
    const float xscr = pos[2] / pos[0] * intrinsics.focal_length_w;
    const float yscr = pos[1] / pos[0] * intrinsics.focal_length_h;
    const float x = (xscr+1.)*0.5f*image_size.width;
    const float y = (yscr+1.)*0.5f*image_size.height;
    return {x, y};
}


quat image_to_world(quat q)
{
    std::swap(q[1], q[3]);
    q[1] = -q[1];
    q[2] = -q[2];
    q[3] = -q[3];
    return q;
}

quat world_to_image(quat q)
{
    // It's its own inverse.
    return image_to_world(q);
}

// Intersection over union. A value between 0 and 1 which measures the match between the bounding boxes.
template<class T>
T iou(const cv::Rect_<T> &a, const cv::Rect_<T> &b)
{
    auto i = a & b;
    return double{i.area()} / (a.area()+b.area()-i.area());
}

// Returns width and height of the input tensor, or throws.
// Expects the model to take one tensor as input that must
// have the shape B x C x H x W, where B=C=1.
cv::Size get_input_image_shape(const Ort::Session &session)
{
    if (session.GetInputCount() < 1)
        throw std::invalid_argument("Model must take at least one input tensor");
    const std::vector<std::int64_t> shape = 
        session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    if (shape.size() != 4)
        throw std::invalid_argument("Model takes the input tensor in the wrong shape");
    return { static_cast<int>(shape[3]), static_cast<int>(shape[2]) };
}


Ort::Value create_tensor(const Ort::TypeInfo& info, Ort::Allocator& alloc)
{
    const auto shape = info.GetTensorTypeAndShapeInfo().GetShape();
    auto t = Ort::Value::CreateTensor<float>(
        alloc, shape.data(), shape.size());
    memset(t.GetTensorMutableData<float>(), 0, sizeof(float)*info.GetTensorTypeAndShapeInfo().GetElementCount());
    return t;
}



int enum_to_fps(int value)
{
    switch (value)
    {
        case fps_30:        return 30;
        case fps_60:        return 60;
        default: [[fallthrough]];
        case fps_default:   return 0;
    }
}


Localizer::Localizer(Ort::MemoryInfo &allocator_info, Ort::Session &&session) :
    session_{std::move(session)},
    scaled_frame_(INPUT_IMG_HEIGHT, INPUT_IMG_WIDTH, CV_8U),
    input_mat_(INPUT_IMG_HEIGHT, INPUT_IMG_WIDTH, CV_32F)
{
    // Only works when input_mat does not reallocated memory ...which it should not.
    // Non-owning memory reference to input_mat?
    // Note: shape = (bach x channels x h x w)
    const std::int64_t input_shape[4] = { 1, 1, INPUT_IMG_HEIGHT, INPUT_IMG_WIDTH };
    input_val_ = Ort::Value::CreateTensor<float>(allocator_info, input_mat_.ptr<float>(0), input_mat_.total(), input_shape, 4);

    const std::int64_t output_shape[2] = { 1, 5 };
    output_val_ = Ort::Value::CreateTensor<float>(allocator_info, results_.data(), results_.size(), output_shape, 2);
}


std::pair<float, cv::Rect2f> Localizer::run(
    const cv::Mat &frame)
{
    auto p = input_mat_.ptr(0);

    cv::resize(frame, scaled_frame_, { INPUT_IMG_WIDTH, INPUT_IMG_HEIGHT }, 0, 0, cv::INTER_AREA);
    scaled_frame_.convertTo(input_mat_, CV_32F, 1./255., -0.5);

    assert (input_mat_.ptr(0) == p);
    assert (!input_mat_.empty() && input_mat_.isContinuous());
    assert (input_mat_.cols == INPUT_IMG_WIDTH && input_mat_.rows == INPUT_IMG_HEIGHT);

    const char* input_names[] = {"x"};
    const char* output_names[] = {"logit_box"};

    Timer t; t.start();

    session_.Run(Ort::RunOptions{nullptr}, input_names, &input_val_, 1, output_names, &output_val_, 1);

    last_inference_time_ = t.elapsed_ms();

    const cv::Rect2f roi = unnormalize(cv::Rect2f{
        results_[1],
        results_[2],
        results_[3]-results_[1], // Width
        results_[4]-results_[2] // Height
    }, frame.rows, frame.cols);
    const float score = sigmoid(results_[0]);

    return { score, roi };
}


double Localizer::last_inference_time_millis() const
{
    return last_inference_time_;
}


PoseEstimator::PoseEstimator(Ort::MemoryInfo &allocator_info, Ort::Session &&_session) 
    : model_version_{_session.GetModelMetadata().GetVersion()}
    , session_{std::move(_session)}
    , allocator_{session_, allocator_info}
{
    using namespace std::literals::string_literals;

    if (session_.GetOutputCount() < 2)
        throw std::runtime_error("Invalid Model: must have at least two outputs");

    // WARNING UB .. but still ...
    // If the model was saved without meta data, it seems the version field is uninitialized.
    // In that case reading from it is UB. However, we will just get same arbitrary number
    // which is hopefully different from the numbers used by models where the version is set.
    // I.e., this is what happended in practice so far.
    if (model_version_ != 2)
        model_version_ = 1;

    const cv::Size input_image_shape = get_input_image_shape(session_);

    scaled_frame_ = cv::Mat(input_image_shape, CV_8U, cv::Scalar(0));
    input_mat_ = cv::Mat(input_image_shape, CV_32F, cv::Scalar(0.f));

    {
        const std::int64_t input_shape[4] = { 1, 1, input_image_shape.height, input_image_shape.width };
        input_val_.push_back(
            Ort::Value::CreateTensor<float>(allocator_info, input_mat_.ptr<float>(0), input_mat_.total(), input_shape, 4));
    }

    {
        const std::int64_t output_shape[2] = { 1, 3 };
        output_val_.push_back(Ort::Value::CreateTensor<float>(
            allocator_info, &output_coord_[0], output_coord_.rows, output_shape, 2));
    }

    {
        const std::int64_t output_shape[2] = { 1, 4 };
        output_val_.push_back(Ort::Value::CreateTensor<float>(
            allocator_info, &output_quat_[0], output_quat_.rows, output_shape, 2));
    }

    size_t num_regular_outputs = 2;

    if (session_.GetOutputCount() >= 3 && "box"s == session_.GetOutputName(2, allocator_))
    {
        const std::int64_t output_shape[2] = { 1, 4 };
        output_val_.push_back(Ort::Value::CreateTensor<float>(
            allocator_info, &output_box_[0], output_box_.rows, output_shape, 2));
        ++num_regular_outputs;
        qDebug() << "Note: Legacy model output for face ROI is currently ignored";
    }

    num_recurrent_states_ = session_.GetInputCount()-1;
    if (session_.GetOutputCount()-num_regular_outputs != num_recurrent_states_)
        throw std::runtime_error("Invalid Model: After regular inputs and outputs the model must have equal number of inputs and outputs for tensors holding hidden states of recurrent layers.");

    // Create tensors for recurrent state
    for (size_t i = 0; i < num_recurrent_states_; ++i)
    {
        const auto& input_info = session_.GetInputTypeInfo(1+i);
        const auto& output_info = session_.GetOutputTypeInfo(num_regular_outputs+i);
        if (input_info.GetTensorTypeAndShapeInfo().GetShape() != 
            output_info.GetTensorTypeAndShapeInfo().GetShape())
            throw std::runtime_error("Invalid Model: Tensors for recurrent hidden states should have same shape on intput and output");
        input_val_.push_back(create_tensor(input_info, allocator_));
        output_val_.push_back(create_tensor(output_info, allocator_));
    }

    for (size_t i = 0; i < session_.GetInputCount(); ++i)
    {
        input_names_.push_back(session_.GetInputName(i, allocator_));
    }
    for (size_t i = 0; i < session_.GetOutputCount(); ++i)
    {
        output_names_.push_back(session_.GetOutputName(i, allocator_));
    }

    qDebug() << "Model inputs: " << session_.GetInputCount() << ", outputs: " << session_.GetOutputCount() << ", recurrent states: " << num_recurrent_states_;

    assert (input_names_.size() == input_val_.size());
    assert (output_names_.size() == output_val_.size());
}


int PoseEstimator::find_input_intensity_90_pct_quantile() const
{
    const int channels[] = { 0 };
    const int hist_size[] = { 255 };
    float range[] = { 0, 256 };
    const float* ranges[] = { range };
    cv::Mat hist;
    cv::calcHist(&scaled_frame_, 1,  channels, cv::Mat(), hist, 1, hist_size, ranges, true, false);
    int gray_level = 0;
    const int num_pixels_quantile = scaled_frame_.total()*0.9f;
    int num_pixels_accum = 0;
    for (int i=0; i<hist_size[0]; ++i)
    {
        num_pixels_accum += hist.at<float>(i);
        if (num_pixels_accum > num_pixels_quantile)
        {
            gray_level = i;
            break;
        }
    }
    return gray_level;
}


std::optional<PoseEstimator::Face> PoseEstimator::run(
    const cv::Mat &frame, const cv::Rect &box)
{
    cv::Mat cropped;

    const int patch_size = std::max(box.width, box.height)*1.05;
    const cv::Point2f patch_center = {
        std::clamp<float>(box.x + 0.5f*box.width, 0.f, frame.cols),
        std::clamp<float>(box.y + 0.5f*box.height, 0.f, frame.rows)
    };
    cv::getRectSubPix(frame, {patch_size, patch_size}, patch_center, cropped);

    // Will get failure if patch_center is outside image boundariesettings.
    // Have to catch this case.
    if (cropped.rows != patch_size || cropped.cols != patch_size)
        return {};

    auto p = input_mat_.ptr(0);

    cv::resize(cropped, scaled_frame_, scaled_frame_.size(), 0, 0, cv::INTER_AREA);

    // Automatic brightness amplification.
    const int brightness = find_input_intensity_90_pct_quantile();
    const double alpha = brightness<127 ? 0.5/std::max(5,brightness) : 1./255;
    const double beta = -0.5;

    scaled_frame_.convertTo(input_mat_, CV_32F, alpha, beta);

    assert (input_mat_.ptr(0) == p);
    assert (!input_mat_.empty() && input_mat_.isContinuous());


    Timer t; t.start();

    try
    {
        session_.Run(
            Ort::RunOptions{ nullptr }, 
            input_names_.data(), 
            input_val_.data(), 
            input_val_.size(), 
            output_names_.data(), 
            output_val_.data(), 
            output_val_.size());
    }
    catch (const Ort::Exception &e)
    {
        qDebug() << "Failed to run the model: " << e.what();
        return {};
    }

    for (size_t i = 0; i<num_recurrent_states_; ++i)
    {
        // Next step, the current output becomes the input.
        // Thus we realize the recurrent connection.
        // Only swaps the internal pointers. There is no copy of data.
        std::swap(
            output_val_[output_val_.size()-num_recurrent_states_+i],
            input_val_[input_val_.size()-num_recurrent_states_+i]);
    }

    // FIXME: Execution time fluctuates wildly. 19 to 26 msec. Why?
    //        The instructions are always the same. Maybe a memory allocation
    //        issue. The ONNX api suggests that tensor are allocated in an
    //        arena. Does that matter? Maybe the issue is something else?

    last_inference_time_ = t.elapsed_ms();

    // Perform coordinate transformation.
    // From patch-local normalized in [-1,1] to
    // frame unnormalized pixel coordinatesettings.

    const cv::Point2f center = patch_center + 
        (0.5f*patch_size)*cv::Point2f{output_coord_[0], output_coord_[1]};

    const float size = patch_size*0.5f*output_coord_[2];

    // Following Eigen which uses quat components in the order w, x, y, z.
    quat rotation = { 
        output_quat_[3], 
        output_quat_[0], 
        output_quat_[1], 
        output_quat_[2] };

    if (model_version_ < 2)
    {
        // Due to a change in coordinate conventions
        rotation = world_to_image(rotation);
    }

    const cv::Rect2f outbox = {
        patch_center.x + (0.5f*patch_size)*output_box_[0],
        patch_center.y + (0.5f*patch_size)*output_box_[1],
        0.5f*patch_size*(output_box_[2]-output_box_[0]),
        0.5f*patch_size*(output_box_[3]-output_box_[1])
    };

    return std::optional<Face>({
        rotation, outbox, center, size
    });
}


cv::Mat PoseEstimator::last_network_input() const
{
    assert(!input_mat_.empty());
    cv::Mat ret;
    input_mat_.convertTo(ret, CV_8U, 255., 127.);
    cv::cvtColor(ret, ret, cv::COLOR_GRAY2RGB);
    return ret;
}


double PoseEstimator::last_inference_time_millis() const
{
    return last_inference_time_;
}


bool NeuralNetTracker::detect()
{
    double inference_time = 0.;

    OnScopeExit update_inference_time{ [&]() {

            QMutexLocker lck{ &stats_mtx_ };
            inference_time_ = inference_time;
    } };

    // If there is no past ROI from the localizer or if the match of its output
    // with the current ROI is too poor we have to run it again. This causes a
    // latency spike of maybe an additional 50%. But it only occurs when the user
    // moves his head far enough - or when the tracking ist lost ...
    if (!last_localizer_roi_ || !last_roi_ ||
        iou(*last_localizer_roi_,*last_roi_)<0.25)
    {
        auto [p, rect] = localizer_->run(grayscale_);
        inference_time += localizer_->last_inference_time_millis();
        
        if (last_roi_ && iou(rect,*last_roi_)>=0.25 && p > 0.5)
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
        draw_gizmos({}, {});
        return false;
    }

    auto face = poseestimator_->run(grayscale_, *last_roi_);
    inference_time += poseestimator_->last_inference_time_millis();

    if (!face)
    {
        last_roi_.reset();
        draw_gizmos(*face, {});
        return false;
    }

    {
        // Here: compute ROI from head size estimate. This helps make the size prediction more
        //       invariant to mouth opening. The tracking can be lost more often at extreme
        //       rotations, depending on the implementation details. The code down here has
        //       been tweaked so that it works pretty well.
        // In old behaviour  ROI is taken from the model outputs
        const vec3 offset = rotate_vec(face->rotation, vec3{0.f, 0.1f*face->size, face->size*0.3f});
        const float halfsize = face->size/float(settings_.roi_zoom);
        face->box = cv::Rect2f(
            face->center.x + offset[0] - halfsize,
            face->center.y + offset[1] - halfsize,
            halfsize*2.f,
            halfsize*2.f
        );
    }

    last_roi_ = ewa_filter(*last_roi_, face->box, float(settings_.roi_filter_alpha));

    Affine pose = compute_pose(*face);

    draw_gizmos(*face, pose);

    {
        QMutexLocker lck(&mtx_);
        this->pose_ = pose;
    }

    return true;
}


void NeuralNetTracker::draw_gizmos(
    const std::optional<PoseEstimator::Face> &face,
    const Affine& pose)
{
    if (!is_visible_)
        return;

    preview_.draw_gizmos(face, pose, last_roi_, last_localizer_roi_, world_to_image(pose.t, grayscale_.size(), intrinsics_));

    if (settings_.show_network_input)
    {
        cv::Mat netinput = poseestimator_->last_network_input();
        preview_.overlay_netinput(netinput);
    }

    //preview_.draw_fps(fps, last_inference_time);
}


Affine NeuralNetTracker::compute_pose(const PoseEstimator::Face &face) const
{
    // Compute the location the network outputs in 3d space.

    const mat33 rot_correction = compute_rotation_correction(
        normalize(face.center, grayscale_.rows, grayscale_.cols),
        intrinsics_.focal_length_w);

    const mat33 m = rot_correction * quaternion_to_mat33(
        image_to_world(face.rotation));

    /*
         
       hhhhhh  <- head size (meters)
      \      | -----------------------
       \     |                         \
        \    |                          |
         \   |                          |- tz (meters)
          ____ <- face.size / width     |
           \ |  |                       |
            \|  |- focal length        /
               ------------------------
    */

    const vec3 face_world_pos = image_to_world(
        face.center.x, face.center.y, face.size, HEAD_SIZE_MM,
        grayscale_.size(),
        intrinsics_);

    // But this is in general not the location of the rotation joint in the neck.
    // So we need an extra offset. Which we determine by solving
    // z,y,z-pos = head_joint_loc + R_face * offset
    const vec3 pos = face_world_pos
        + m * vec3{
            static_cast<float>(settings_.offset_fwd),
            static_cast<float>(settings_.offset_up),
            static_cast<float>(settings_.offset_right)};

    return { m, pos };
}


void Preview::init(const cv_video_widget& widget)
{
    auto [w,h] = widget.preview_size();
    preview_size_ = { w, h };
}


void Preview::copy_video_frame(const cv::Mat& frame)
{
    cv::Rect roi = make_crop_rect_for_aspect(frame.size(), preview_size_.width, preview_size_.height);
    
    cv::resize(frame(roi), preview_image_, preview_size_, 0, 0, cv::INTER_NEAREST);

    offset_ = { (float)-roi.x, (float)-roi.y };
    scale_ = float(preview_image_.cols) / float(roi.width);
}


void Preview::draw_gizmos(
    const std::optional<PoseEstimator::Face> &face,
    const Affine& pose,
    const std::optional<cv::Rect2f>& last_roi,
    const std::optional<cv::Rect2f>& last_localizer_roi,
    const cv::Point2f& neckjoint_position)
{
    if (preview_image_.empty())
        return;

    if (last_roi) 
    {
        const int col = 255;
        cv::rectangle(preview_image_, transform(*last_roi), cv::Scalar(0, col, 0), /*thickness=*/1);
    }
    if (last_localizer_roi)
    {
        const int col = 255;
        cv::rectangle(preview_image_, transform(*last_localizer_roi), cv::Scalar(col, 0, 255-col), /*thickness=*/1);
    }

    if (face)
    {
        if (face->size>=1.f)
            cv::circle(preview_image_, static_cast<cv::Point>(transform(face->center)), int(transform(face->size)), cv::Scalar(255,255,255), 2);
        cv::circle(preview_image_, static_cast<cv::Point>(transform(face->center)), 3, cv::Scalar(255,255,255), -1);

        auto draw_coord_line = [&](int i, const cv::Scalar& color)
        {
            const float vx = -pose.R(2,i);
            const float vy = -pose.R(1,i);
            static constexpr float len = 100.f;
            cv::Point q = face->center + len*cv::Point2f{vx, vy};
            cv::line(preview_image_, static_cast<cv::Point>(transform(face->center)), static_cast<cv::Point>(transform(q)), color, 2);
        };
        draw_coord_line(0, {0, 0, 255});
        draw_coord_line(1, {0, 255, 0});
        draw_coord_line(2, {255, 0, 0});

        // Draw the computed joint position
        auto xy = transform(neckjoint_position);
        cv::circle(preview_image_, cv::Point(xy.x,xy.y), 5, cv::Scalar(0,0,255), -1);
    }
}

void Preview::overlay_netinput(const cv::Mat& netinput)
{
    if (netinput.empty())
        return;

    const int w = std::min(netinput.cols, preview_image_.cols);
    const int h = std::min(netinput.rows, preview_image_.rows);
    cv::Rect roi(0, 0, w, h);
    netinput(roi).copyTo(preview_image_(roi));
}

void Preview::draw_fps(double fps, double last_inference_time)
{
    char buf[128];
    ::snprintf(buf, sizeof(buf), "%d Hz, pose inference: %d ms", std::clamp(int(fps), 0, 9999), int(last_inference_time));
    cv::putText(preview_image_, buf, cv::Point(10, preview_image_.rows-10), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0), 1);
}


void Preview::copy_to_widget(cv_video_widget& widget)
{
    if (preview_image_.rows > 0)
        widget.update_image(preview_image_);
}


cv::Rect2f Preview::transform(const cv::Rect2f& r) const
{
    return { (r.x - offset_.x)*scale_, (r.y - offset_.y)*scale_, r.width*scale_, r.height*scale_ };
}

cv::Point2f Preview::transform(const cv::Point2f& p) const
{
    return { (p.x - offset_.x)*scale_ , (p.y - offset_.y)*scale_ };
}

float Preview::transform(float s) const
{
    return s * scale_;
}


NeuralNetTracker::NeuralNetTracker()
{
    opencv_init();
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
    videoframe->show();
    video_widget_ = std::make_unique<cv_video_widget>(videoframe);
    layout_ = std::make_unique<QHBoxLayout>();
    layout_->setContentsMargins(0, 0, 0, 0);
    layout_->addWidget(&*video_widget_);
    videoframe->setLayout(&*layout_);
    video_widget_->show();
    num_threads_ = settings_.num_threads;
    start();
    return status_ok();
}


bool NeuralNetTracker::load_and_initialize_model()
{
    const QString localizer_model_path_enc =
        OPENTRACK_BASE_PATH+"/" OPENTRACK_LIBRARY_PATH "/models/head-localizer.onnx";
    const QString poseestimator_model_path_enc =
        OPENTRACK_BASE_PATH+"/" OPENTRACK_LIBRARY_PATH "/models/head-pose.onnx";

    try
    {
        env_ = Ort::Env{
            OrtLoggingLevel::ORT_LOGGING_LEVEL_ERROR,
            "tracker-neuralnet"
        };
        auto opts = Ort::SessionOptions{};
        // Do thread settings here do anything?
        // There is a warning which says to control number of threads via
        // openmp settings. Which is what we do.
        opts.SetIntraOpNumThreads(num_threads_);
        opts.SetInterOpNumThreads(1);
        allocator_info_ = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        localizer_.emplace(
            allocator_info_, 
            Ort::Session{env_, convert(localizer_model_path_enc).c_str(), opts});

        poseestimator_.emplace(
            allocator_info_,
            Ort::Session{env_, convert(poseestimator_model_path_enc).c_str(), opts});
    }
    catch (const Ort::Exception &e)
    {
        qDebug() << "Failed to initialize the neural network models. ONNX error message: " 
            << e.what();
        return false;
    }
    return true;
}


bool NeuralNetTracker::open_camera()
{
    int rint = std::clamp(*settings_.resolution, 0, (int)std::size(resolution_choices)-1);
    resolution_tuple res = resolution_choices[rint];
    int fps = enum_to_fps(settings_.force_fps);

    QMutexLocker l(&camera_mtx_);

    camera_ = video::make_camera(settings_.camera_name);

    if (!camera_)
        return false;

    video::impl::camera::info args {};

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


void NeuralNetTracker::set_intrinsics()
{
    const int w = grayscale_.cols, h = grayscale_.rows;
    const double diag_fov = settings_.fov * M_PI / 180.;
    const double fov_w = 2.*atan(tan(diag_fov/2.)/sqrt(1. + h/(double)w * h/(double)w));
    const double fov_h = 2.*atan(tan(diag_fov/2.)/sqrt(1. + w/(double)h * w/(double)h));
    const double focal_length_w = 1. / tan(.5 * fov_w);
    const double focal_length_h = 1. / tan(.5 * fov_h);

    intrinsics_.fov_h = fov_h;
    intrinsics_.fov_w = fov_w;
    intrinsics_.focal_length_w = focal_length_w;
    intrinsics_.focal_length_h = focal_length_h;
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


void NeuralNetTracker::run()
{
    preview_.init(*video_widget_);

    GuardedThreadCountSwitch switch_num_threads_to(num_threads_);

    if (!open_camera())
        return;

    if (!load_and_initialize_model())
        return;

    std::chrono::high_resolution_clock clk;

    while (!isInterruptionRequested())
    {
        is_visible_ = check_is_visible();
        auto t = clk.now();
        {
            QMutexLocker l(&camera_mtx_);

            auto [ img, res ] = camera_->get_frame();

            if (!res)
            {
                l.unlock();
                portable::sleep(100);
                continue;
            }

            {
                QMutexLocker lck{&stats_mtx_};
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

        set_intrinsics();

        detect();

        if (is_visible_)
            preview_.copy_to_widget(*video_widget_);
        
        update_fps(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                clk.now() - t).count()*1.e-3);
    }
}


cv::Mat NeuralNetTracker::prepare_input_image(const video::frame& frame)
{
    auto img = cv::Mat(frame.height, frame.width, CV_8UC(frame.channels), (void*)frame.data, frame.stride);

    // Crop if aspect ratio is not 4:3
    if (img.rows*4 != img.cols*3)
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
    const double alpha = dt/(dt + RC);
    if (dt > 1e-6)
    {
        QMutexLocker lck{&stats_mtx_};
        fps_ *= 1 - alpha;
        fps_ += alpha * 1./dt;
    }
}


void NeuralNetTracker::data(double *data)
{
    Affine tmp = [&]()
    {
        QMutexLocker lck(&mtx_);
        return pose_;
    }();

    const auto& mx = tmp.R.col(0);
    const auto& my = tmp.R.col(1);
    const auto& mz = -tmp.R.col(2);

    const float yaw = std::atan2(mx(2), mx(0));
    const float pitch = -std::atan2(-mx(1), std::sqrt(mx(2)*mx(2)+mx(0)*mx(0)));
    const float roll = std::atan2(-my(2), mz(2));
    {
        constexpr double rad2deg = 180/M_PI;
        data[Yaw]   = rad2deg * yaw;
        data[Pitch] = rad2deg * pitch;
        data[Roll]  = rad2deg * roll;

        // convert to cm
        data[TX] = -tmp.t[2] * 0.1;
        data[TY] = tmp.t[1] * 0.1;
        data[TZ] = -tmp.t[0] * 0.1;
    }
}


Affine NeuralNetTracker::pose()
{
    QMutexLocker lck(&mtx_);
    return pose_;
}

std::tuple<cv::Size,double, double> NeuralNetTracker::stats() const
{
    QMutexLocker lck(&stats_mtx_);
    return { resolution_, fps_, inference_time_ };
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
    int k=0;
    for (const auto [w, h] : resolution_choices)
    {
        const QString s = (w == 0) 
            ? tr("Default") 
            : QString::number(w) + " x " + QString::number(h);
        ui_.resolution->addItem(s, k++);
    }
}


NeuralNetDialog::NeuralNetDialog() :
    trans_calib_(1, 2)
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

    connect(ui_.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui_.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui_.camera_settings, SIGNAL(clicked()), this, SLOT(camera_settings()));

    settings_.camera_name.connect_to(this, &NeuralNetDialog::update_camera_settings_state);

    update_camera_settings_state(settings_.camera_name);

    connect(&calib_timer_, &QTimer::timeout, this, &NeuralNetDialog::trans_calib_step);
    calib_timer_.setInterval(35);
    connect(ui_.tcalib_button,SIGNAL(toggled(bool)), this, SLOT(startstop_trans_calib(bool)));

    connect(&tracker_status_poll_timer_, &QTimer::timeout, this, &NeuralNetDialog::status_poll);
    tracker_status_poll_timer_.setInterval(250);
    tracker_status_poll_timer_.start();
}


void NeuralNetDialog::doOK()
{
    settings_.b->save();
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


void NeuralNetDialog::register_tracker(ITracker * x)
{
    tracker_ = static_cast<NeuralNetTracker*>(x);
    ui_.tcalib_button->setEnabled(true);
}


void NeuralNetDialog::unregister_tracker()
{
    tracker_ = nullptr;
    ui_.tcalib_button->setEnabled(false);
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
        auto [ res, fps, inference_time ] = tracker_->stats();
        status = tr("%1x%2 @ %3 FPS / Inference: %4 ms").arg(res.width).arg(res.height).arg(int(fps)).arg(int(inference_time));
    }
    ui_.resolution_display->setText(status);
}


void NeuralNetDialog::trans_calib_step()
{
    if (tracker_)
    {
        const Affine X_CM = [&]() { 
            QMutexLocker l(&calibrator_mutex_);
            return tracker_->pose();
        }();
        trans_calib_.update(X_CM.R, X_CM.t);
        auto [_, nsamples] = trans_calib_.get_estimate();

        constexpr int min_yaw_samples = 15;
        constexpr int min_pitch_samples = 12;
        constexpr int min_samples = min_yaw_samples+min_pitch_samples;

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


Settings::Settings() : opts("neuralnet-tracker") {}

} // neuralnet_tracker_ns

OPENTRACK_DECLARE_TRACKER(NeuralNetTracker, NeuralNetDialog, NeuralNetMetadata)
