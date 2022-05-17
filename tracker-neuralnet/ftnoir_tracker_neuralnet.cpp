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
    session{std::move(session)},
    scaled_frame(input_img_height, input_img_width, CV_8U),
    input_mat(input_img_height, input_img_width, CV_32F)
{
    // Only works when input_mat does not reallocated memory ...which it should not.
    // Non-owning memory reference to input_mat?
    // Note: shape = (bach x channels x h x w)
    const std::int64_t input_shape[4] = { 1, 1, input_img_height, input_img_width };
    input_val = Ort::Value::CreateTensor<float>(allocator_info, input_mat.ptr<float>(0), input_mat.total(), input_shape, 4);

    const std::int64_t output_shape[2] = { 1, 5 };
    output_val = Ort::Value::CreateTensor<float>(allocator_info, results.data(), results.size(), output_shape, 2);
}


std::pair<float, cv::Rect2f> Localizer::run(
    const cv::Mat &frame)
{
    auto p = input_mat.ptr(0);

    cv::resize(frame, scaled_frame, { input_img_width, input_img_height }, 0, 0, cv::INTER_AREA);
    scaled_frame.convertTo(input_mat, CV_32F, 1./255., -0.5);

    assert (input_mat.ptr(0) == p);
    assert (!input_mat.empty() && input_mat.isContinuous());
    assert (input_mat.cols == input_img_width && input_mat.rows == input_img_height);

    const char* input_names[] = {"x"};
    const char* output_names[] = {"logit_box"};

    Timer t; t.start();

    session.Run(Ort::RunOptions{nullptr}, input_names, &input_val, 1, output_names, &output_val, 1);

    last_inference_time = t.elapsed_ms();

    const cv::Rect2f roi = unnormalize(cv::Rect2f{
        results[1],
        results[2],
        results[3]-results[1], // Width
        results[4]-results[2] // Height
    }, frame.rows, frame.cols);
    const float score = sigmoid(results[0]);

    return { score, roi };
}


double Localizer::last_inference_time_millis() const
{
    return last_inference_time;
}


PoseEstimator::PoseEstimator(Ort::MemoryInfo &allocator_info, Ort::Session &&_session) 
    : model_version{_session.GetModelMetadata().GetVersion()}
    , session{std::move(_session)}
    , allocator{session, allocator_info}
{
    using namespace std::literals::string_literals;

    if (session.GetOutputCount() < 2)
        throw std::runtime_error("Invalid Model: must have at least two outputs");

    // WARNING
    // If the model was saved without meta data, it seems the version field is uninitialized.
    // In that case reading from it is UB. However, we will just get same arbitrary number
    // which is hopefully different from the numbers used by models where the version is set.
    if (model_version != 2)
        model_version = 1;

    const cv::Size input_image_shape = get_input_image_shape(session);

    scaled_frame = cv::Mat(input_image_shape, CV_8U);
    input_mat = cv::Mat(input_image_shape, CV_32F); 

    {
        const std::int64_t input_shape[4] = { 1, 1, input_image_shape.height, input_image_shape.width };
        input_val.push_back(
            Ort::Value::CreateTensor<float>(allocator_info, input_mat.ptr<float>(0), input_mat.total(), input_shape, 4));
    }

    {
        const std::int64_t output_shape[2] = { 1, 3 };
        output_val.push_back(Ort::Value::CreateTensor<float>(
            allocator_info, &output_coord[0], output_coord.rows, output_shape, 2));
    }

    {
        const std::int64_t output_shape[2] = { 1, 4 };
        output_val.push_back(Ort::Value::CreateTensor<float>(
            allocator_info, &output_quat[0], output_quat.rows, output_shape, 2));
    }

    size_t num_regular_outputs = 2;

    if (session.GetOutputCount() >= 3 && "box"s == session.GetOutputName(2, allocator))
    {
        const std::int64_t output_shape[2] = { 1, 4 };
        output_val.push_back(Ort::Value::CreateTensor<float>(
            allocator_info, &output_box[0], output_box.rows, output_shape, 2));
        ++num_regular_outputs;
        qDebug() << "Note: Legacy model output for face ROI is currently ignored";
    }

    num_recurrent_states = session.GetInputCount()-1;
    if (session.GetOutputCount()-num_regular_outputs != num_recurrent_states)
        throw std::runtime_error("Invalid Model: After regular inputs and outputs the model must have equal number of inputs and outputs for tensors holding hidden states of recurrent layers.");

    // Create tensors for recurrent state
    for (size_t i = 0; i < num_recurrent_states; ++i)
    {
        const auto& input_info = session.GetInputTypeInfo(1+i);
        const auto& output_info = session.GetOutputTypeInfo(num_regular_outputs+i);
        if (input_info.GetTensorTypeAndShapeInfo().GetShape() != 
            output_info.GetTensorTypeAndShapeInfo().GetShape())
            throw std::runtime_error("Invalid Model: Tensors for recurrent hidden states should have same shape on intput and output");
        input_val.push_back(create_tensor(input_info, allocator));
        output_val.push_back(create_tensor(output_info, allocator));
    }

    for (size_t i = 0; i < session.GetInputCount(); ++i)
    {
        input_names.push_back(session.GetInputName(i, allocator));
    }
    for (size_t i = 0; i < session.GetOutputCount(); ++i)
    {
        output_names.push_back(session.GetOutputName(i, allocator));
    }

    qDebug() << "Model inputs: " << session.GetInputCount() << ", outputs: " << session.GetOutputCount() << ", recurrent states: " << num_recurrent_states;

    assert (input_names.size() == input_val.size());
    assert (output_names.size() == output_val.size());
}


int PoseEstimator::find_input_intensity_90_pct_quantile() const
{
    const int channels[] = { 0 };
    const int hist_size[] = { 255 };
    float range[] = { 0, 256 };
    const float* ranges[] = { range };
    cv::Mat hist;
    cv::calcHist(&scaled_frame, 1,  channels, cv::Mat(), hist, 1, hist_size, ranges, true, false);
    int gray_level = 0;
    const int num_pixels_quantile = scaled_frame.total()*0.9f;
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

    auto p = input_mat.ptr(0);

    cv::resize(cropped, scaled_frame, scaled_frame.size(), 0, 0, cv::INTER_AREA);

    // Automatic brightness amplification.
    const int brightness = find_input_intensity_90_pct_quantile();
    const double alpha = brightness<127 ? 0.5/std::max(5,brightness) : 1./255;
    const double beta = -0.5;

    scaled_frame.convertTo(input_mat, CV_32F, alpha, beta);

    assert (input_mat.ptr(0) == p);
    assert (!input_mat.empty() && input_mat.isContinuous());


    Timer t; t.start();

    try
    {
        session.Run(
            Ort::RunOptions{ nullptr }, 
            input_names.data(), 
            input_val.data(), 
            input_val.size(), 
            output_names.data(), 
            output_val.data(), 
            output_val.size());
    }
    catch (const Ort::Exception &e)
    {
        qDebug() << "Failed to run the model: " << e.what();
        return {};
    }

    for (size_t i = 0; i<num_recurrent_states; ++i)
    {
        // Next step, the current output becomes the input.
        // Thus we realize the recurrent connection.
        // Only swaps the internal pointers. There is no copy of data.
        std::swap(
            output_val[output_val.size()-num_recurrent_states+i],
            input_val[input_val.size()-num_recurrent_states+i]);
    }

    // FIXME: Execution time fluctuates wildly. 19 to 26 msec. Why?
    //        The instructions are always the same. Maybe a memory allocation
    //        issue. The ONNX api suggests that tensor are allocated in an
    //        arena. Does that matter? Maybe the issue is something else?

    last_inference_time = t.elapsed_ms();

    // Perform coordinate transformation.
    // From patch-local normalized in [-1,1] to
    // frame unnormalized pixel coordinatesettings.

    const cv::Point2f center = patch_center + 
        (0.5f*patch_size)*cv::Point2f{output_coord[0], output_coord[1]};

    const float size = patch_size*0.5f*output_coord[2];

    // Following Eigen which uses quat components in the order w, x, y, z.
    quat rotation = { 
        output_quat[3], 
        output_quat[0], 
        output_quat[1], 
        output_quat[2] };

    if (model_version < 2)
    {
        // Due to a change in coordinate conventions
        rotation = world_to_image(rotation);
    }

    const cv::Rect2f outbox = {
        patch_center.x + (0.5f*patch_size)*output_box[0],
        patch_center.y + (0.5f*patch_size)*output_box[1],
        0.5f*patch_size*(output_box[2]-output_box[0]),
        0.5f*patch_size*(output_box[3]-output_box[1])
    };

    return std::optional<Face>({
        rotation, outbox, center, size
    });
}


cv::Mat PoseEstimator::last_network_input() const
{
    cv::Mat ret;
    if (!input_mat.empty())
    {
        input_mat.convertTo(ret, CV_8U, 255., 127.);
        cv::cvtColor(ret, ret, cv::COLOR_GRAY2RGB);
    }
    return ret;
}


double PoseEstimator::last_inference_time_millis() const
{
    return last_inference_time;
}


bool neuralnet_tracker::detect()
{
    // Note: BGR colors!
    if (!last_localizer_roi || !last_roi ||
        iou(*last_localizer_roi,*last_roi)<0.25)
    {
        auto [p, rect] = localizer->run(grayscale);
        last_inference_time += localizer->last_inference_time_millis();
        if (p > 0.5 || rect.height < 5 || rect.width < 5)
        {
            last_localizer_roi = rect;
            last_roi = rect;
        }
        else
        {
            last_roi.reset();
            last_localizer_roi.reset();
        }
    }

    if (!last_roi)
    {
        draw_gizmos(frame, {}, {});
        return false;
    }

    auto face = poseestimator->run(grayscale, *last_roi);
    last_inference_time += poseestimator->last_inference_time_millis();

    if (!face)
    {
        last_roi.reset();
        draw_gizmos(frame, *face, {});
        return false;
    }

    {
        // Here: compute ROI from head size estimate. This helps make the size prediction more
        //       invariant to mouth opening. The tracking can be lost more often at extreme
        //       rotations, depending on the implementation details. The code down here has
        //       been tweaked so that it works pretty well.
        // In old behaviour  ROI is taken from the model outputs
        const vec3 offset = rotate_vec(face->rotation, vec3{0.f, 0.1f*face->size, face->size*0.3f});
        const float halfsize = face->size/float(settings.roi_zoom);
        face->box = cv::Rect2f(
            face->center.x + offset[0] - halfsize,
            face->center.y + offset[1] - halfsize,
            halfsize*2.f,
            halfsize*2.f
        );
    }

    last_roi = ewa_filter(*last_roi, face->box, float(settings.roi_filter_alpha));

    Affine pose = compute_pose(*face);

    draw_gizmos(frame, *face, pose);

    {
        QMutexLocker lck(&mtx);
        this->pose_ = pose;
    }

    return true;
}


Affine neuralnet_tracker::compute_pose(const PoseEstimator::Face &face) const
{
    // Compute the location the network outputs in 3d space.

    const mat33 rot_correction = compute_rotation_correction(
        normalize(face.center, frame.rows, frame.cols),
        intrinsics.focal_length_w);

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
        face.center.x, face.center.y, face.size, head_size_mm,
        frame.size(),
        intrinsics);

    // But this is in general not the location of the rotation joint in the neck.
    // So we need an extra offset. Which we determine by solving
    // z,y,z-pos = head_joint_loc + R_face * offset

    const vec3 pos = face_world_pos
        + m * vec3{
            static_cast<float>(settings.offset_fwd), 
            static_cast<float>(settings.offset_up),
            static_cast<float>(settings.offset_right)};

    return { m, pos };
}


void neuralnet_tracker::draw_gizmos(
    cv::Mat frame,
    const std::optional<PoseEstimator::Face> &face,
    const Affine& pose) const
{
    if (last_roi) 
    {
        const int col = 255;
        cv::rectangle(frame, *last_roi, cv::Scalar(0, col, 0), /*thickness=*/1);
    }
    if (last_localizer_roi)
    {
        const int col = 255;
        cv::rectangle(frame, *last_localizer_roi, cv::Scalar(col, 0, 255-col), /*thickness=*/1);
    }

    if (face)
    {
        if (face->size>=1.f)
            cv::circle(frame, static_cast<cv::Point>(face->center), int(face->size), cv::Scalar(255,255,255), 2);
        cv::circle(frame, static_cast<cv::Point>(face->center), 3, cv::Scalar(255,255,255), -1);

        auto draw_coord_line = [&](int i, const cv::Scalar& color)
        {
            const float vx = -pose.R(2,i);
            const float vy = -pose.R(1,i);
            static constexpr float len = 100.f;
            cv::Point q = face->center + len*cv::Point2f{vx, vy};
            cv::line(frame, static_cast<cv::Point>(face->center), static_cast<cv::Point>(q), color, 2);
        };
        draw_coord_line(0, {0, 0, 255});
        draw_coord_line(1, {0, 255, 0});
        draw_coord_line(2, {255, 0, 0});

        // Draw the computed joint position
        auto xy = world_to_image(pose.t, frame.size(), intrinsics);
        cv::circle(frame, cv::Point(xy[0],xy[1]), 5, cv::Scalar(0,0,255), -1);
    }

    if (settings.show_network_input)
    {
        cv::Mat netinput = poseestimator->last_network_input();
        if (!netinput.empty())
        {
            const int w = std::min(netinput.cols, frame.cols);
            const int h = std::min(netinput.rows, frame.rows);
            cv::Rect roi(0, 0, w, h);
            netinput(roi).copyTo(frame(roi));
        }
    }

    char buf[128];
    ::snprintf(buf, sizeof(buf), "%d Hz, pose inference: %d ms", std::clamp(int(fps), 0, 9999), int(last_inference_time));
    cv::putText(frame, buf, cv::Point(10, frame.rows-10), cv::FONT_HERSHEY_PLAIN, 1, cv::Scalar(0, 255, 0), 1);
}


neuralnet_tracker::neuralnet_tracker()
{
    opencv_init();
}


neuralnet_tracker::~neuralnet_tracker()
{
    requestInterruption();
    wait();
    // fast start/stop causes breakage
    portable::sleep(1000);
}


module_status neuralnet_tracker::start_tracker(QFrame* videoframe)
{
    videoframe->show();
    videoWidget = std::make_unique<cv_video_widget>(videoframe);
    layout = std::make_unique<QHBoxLayout>();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(&*videoWidget);
    videoframe->setLayout(&*layout);
    videoWidget->show();
    num_threads = settings.num_threads;
    start();
    return status_ok();
}


bool neuralnet_tracker::load_and_initialize_model()
{
    const QString localizer_model_path_enc =
        OPENTRACK_BASE_PATH+"/" OPENTRACK_LIBRARY_PATH "/models/head-localizer.onnx";
    const QString poseestimator_model_path_enc =
        OPENTRACK_BASE_PATH+"/" OPENTRACK_LIBRARY_PATH "/models/head-pose.onnx";

    try
    {
        env = Ort::Env{
            OrtLoggingLevel::ORT_LOGGING_LEVEL_ERROR,
            "tracker-neuralnet"
        };
        auto opts = Ort::SessionOptions{};
        // Do thread settings here do anything?
        // There is a warning which says to control number of threads via
        // openmp settings. Which is what we do.
        opts.SetIntraOpNumThreads(num_threads);
        opts.SetInterOpNumThreads(1);
        allocator_info = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);

        localizer.emplace(
            allocator_info, 
            Ort::Session{env, convert(localizer_model_path_enc).c_str(), opts});

        poseestimator.emplace(
            allocator_info,
            Ort::Session{env, convert(poseestimator_model_path_enc).c_str(), opts});
    }
    catch (const Ort::Exception &e)
    {
        qDebug() << "Failed to initialize the neural network models. ONNX error message: " 
            << e.what();
        return false;
    }
    return true;
}


bool neuralnet_tracker::open_camera()
{
    int rint = std::clamp(*settings.resolution, 0, (int)std::size(resolution_choices)-1);
    resolution_tuple res = resolution_choices[rint];
    int fps = enum_to_fps(settings.force_fps);

    QMutexLocker l(&camera_mtx);

    camera = video::make_camera(settings.camera_name);

    if (!camera)
        return false;

    video::impl::camera::info args {};

    if (res.width)
    {
        args.width = res.width;
        args.height = res.height;
    }
    if (fps)
        args.fps = fps;

    args.use_mjpeg = settings.use_mjpeg;

    if (!camera->start(args))
    {
        qDebug() << "neuralnet tracker: can't open camera";
        return false;
    }
    return true;
}


void neuralnet_tracker::set_intrinsics()
{
    const int w = grayscale.cols, h = grayscale.rows;
    const double diag_fov = settings.fov * M_PI / 180.;
    const double fov_w = 2.*atan(tan(diag_fov/2.)/sqrt(1. + h/(double)w * h/(double)w));
    const double fov_h = 2.*atan(tan(diag_fov/2.)/sqrt(1. + w/(double)h * w/(double)h));
    const double focal_length_w = 1. / tan(.5 * fov_w);
    const double focal_length_h = 1. / tan(.5 * fov_h);

    intrinsics.fov_h = fov_h;
    intrinsics.fov_w = fov_w;
    intrinsics.focal_length_w = focal_length_w;
    intrinsics.focal_length_h = focal_length_h;
}


class GuardedThreadCountSwitch
{
    int old_num_threads_cv = 1;
    int old_num_threads_omp = 1;
    public:
        GuardedThreadCountSwitch(int num_threads)
        {
            old_num_threads_cv = cv::getNumThreads();
            old_num_threads_omp = omp_get_num_threads();
            omp_set_num_threads(num_threads);
            cv::setNumThreads(num_threads);
        }
            
        ~GuardedThreadCountSwitch()
        {
            omp_set_num_threads(old_num_threads_omp);
            cv::setNumThreads(old_num_threads_cv);
        }

        GuardedThreadCountSwitch(const GuardedThreadCountSwitch&) = delete;
        GuardedThreadCountSwitch& operator=(const GuardedThreadCountSwitch&) = delete;
};


void neuralnet_tracker::run()
{
    GuardedThreadCountSwitch switch_num_threads_to(num_threads);

    if (!open_camera())
        return;

    if (!load_and_initialize_model())
        return;

    std::chrono::high_resolution_clock clk;

    while (!isInterruptionRequested())
    {
        last_inference_time = 0;
        auto t = clk.now();
        {
            QMutexLocker l(&camera_mtx);

            auto [ img, res ] = camera->get_frame();

            if (!res)
            {
                l.unlock();
                portable::sleep(100);
                continue;
            }

            auto color = prepare_input_image(img);

            color.copyTo(frame);

            switch (img.channels)
            {
            case 1:
                grayscale.create(img.height, img.width, CV_8UC1);
                color.copyTo(grayscale);
                break;
            case 3:
                cv::cvtColor(color, grayscale, cv::COLOR_BGR2GRAY);
                break;
            default:
                qDebug() << "Can't handle" << img.channels << "color channels";
                return;
            }
        }

        set_intrinsics();

        detect();

        if (frame.rows > 0)
            videoWidget->update_image(frame);
        
        update_fps(
            std::chrono::duration_cast<std::chrono::milliseconds>(
                clk.now() - t).count()*1.e-3);
    }
}


cv::Mat neuralnet_tracker::prepare_input_image(const video::frame& frame)
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


void neuralnet_tracker::update_fps(double dt)
{
    const double alpha = dt/(dt + RC);

    if (dt > 1e-6)
    {
        fps *= 1 - alpha;
        fps += alpha * 1./dt;
    }
}


void neuralnet_tracker::data(double *data)
{
    Affine tmp = [&]()
    {
        QMutexLocker lck(&mtx);
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


Affine neuralnet_tracker::pose()
{
    QMutexLocker lck(&mtx);
    return pose_;
}


void neuralnet_dialog::make_fps_combobox()
{
    for (int k = 0; k < fps_MAX; k++)
    {
        const int hz = enum_to_fps(k);
        const QString name = (hz == 0) ? tr("Default") : QString::number(hz);
        ui.cameraFPS->addItem(name, k);
    }
}

void neuralnet_dialog::make_resolution_combobox()
{
    int k=0;
    for (const auto [w, h] : resolution_choices)
    {
        const QString s = (w == 0) 
            ? tr("Default") 
            : QString::number(w) + " x " + QString::number(h);
        ui.resolution->addItem(s, k++);
    }
}


neuralnet_dialog::neuralnet_dialog() :
    trans_calib(1, 2)
{
    ui.setupUi(this);

    make_fps_combobox();
    make_resolution_combobox();

    for (const auto& str : video::camera_names())
        ui.cameraName->addItem(str);

    tie_setting(settings.camera_name, ui.cameraName);
    tie_setting(settings.fov, ui.cameraFOV);
    tie_setting(settings.offset_fwd, ui.tx_spin);
    tie_setting(settings.offset_up, ui.ty_spin);
    tie_setting(settings.offset_right, ui.tz_spin);
    tie_setting(settings.show_network_input, ui.showNetworkInput);
    tie_setting(settings.roi_filter_alpha, ui.roiFilterAlpha);
    tie_setting(settings.use_mjpeg, ui.use_mjpeg);
	tie_setting(settings.roi_zoom, ui.roiZoom);
    tie_setting(settings.num_threads, ui.threadCount);
    tie_setting(settings.resolution, ui.resolution);
    tie_setting(settings.force_fps, ui.cameraFPS);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.camera_settings, SIGNAL(clicked()), this, SLOT(camera_settings()));

    connect(&settings.camera_name, value_::value_changed<QString>(), this, &neuralnet_dialog::update_camera_settings_state);

    update_camera_settings_state(settings.camera_name);

    connect(&calib_timer, &QTimer::timeout, this, &neuralnet_dialog::trans_calib_step);
    calib_timer.setInterval(35);
    connect(ui.tcalib_button,SIGNAL(toggled(bool)), this, SLOT(startstop_trans_calib(bool)));
}


void neuralnet_dialog::doOK()
{
    settings.b->save();
    close();
}


void neuralnet_dialog::doCancel()
{
    close();
}


void neuralnet_dialog::camera_settings()
{
    if (tracker)
    {
        QMutexLocker l(&tracker->camera_mtx);
        (void)tracker->camera->show_dialog();
    }
    else
        (void)video::show_dialog(settings.camera_name);
}


void neuralnet_dialog::update_camera_settings_state(const QString& name)
{
    (void)name;
    ui.camera_settings->setEnabled(true);
}


void neuralnet_dialog::register_tracker(ITracker * x)
{
    tracker = static_cast<neuralnet_tracker*>(x);
    ui.tcalib_button->setEnabled(true);
}


void neuralnet_dialog::unregister_tracker()
{
    tracker = nullptr;
    ui.tcalib_button->setEnabled(false);
}


void neuralnet_dialog::trans_calib_step()
{
    if (tracker)
    {
        const Affine X_CM = [&]() { 
            QMutexLocker l(&calibrator_mutex);
            return tracker->pose();
        }();
        trans_calib.update(X_CM.R, X_CM.t);
        auto [_, nsamples] = trans_calib.get_estimate();

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
        ui.sample_count_display->setText(sample_feedback);
    }
    else
        startstop_trans_calib(false);
}


void neuralnet_dialog::startstop_trans_calib(bool start)
{
    QMutexLocker l(&calibrator_mutex);
    // FIXME: does not work ...  
    if (start)
    {
        qDebug() << "pt: starting translation calibration";
        calib_timer.start();
        trans_calib.reset();
        ui.sample_count_display->setText(QString());
        // Tracker must run with zero'ed offset for calibration.
        settings.offset_fwd = 0;
        settings.offset_up = 0;
        settings.offset_right = 0;
    }
    else
    {
        calib_timer.stop();
        qDebug() << "pt: stopping translation calibration";
        {
            auto [tmp, nsamples] = trans_calib.get_estimate();
            settings.offset_fwd = int(tmp[0]);
            settings.offset_up = int(tmp[1]);
            settings.offset_right = int(tmp[2]);
        }
    }
    ui.tx_spin->setEnabled(!start);
    ui.ty_spin->setEnabled(!start);
    ui.tz_spin->setEnabled(!start);

    if (start)
        ui.tcalib_button->setText(tr("Stop calibration"));
    else
        ui.tcalib_button->setText(tr("Start calibration"));
}


Settings::Settings() : opts("neuralnet-tracker") {}

} // neuralnet_tracker_ns

OPENTRACK_DECLARE_TRACKER(neuralnet_tracker, neuralnet_dialog, neuralnet_metadata)
