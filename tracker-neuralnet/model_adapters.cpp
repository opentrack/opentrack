#include "model_adapters.h"

#include "compat/timer.hpp"

#include <opencv2/core.hpp>
#include <opencv2/core/quaternion.hpp>
#include <opencv2/imgproc.hpp>

#include <QDebug>

namespace neuralnet_tracker_ns
{

float sigmoid(float x)
{
    return 1.f / (1.f + std::exp(-x));
}

// Defined in ftnoir_tracker_neuralnet.cpp
// Normally we wouldn't need it here. However ... see below.
cv::Quatf image_to_world(cv::Quatf q);

cv::Quatf world_to_image(cv::Quatf q)
{
    // It's its own inverse.
    return image_to_world(q);
}

cv::Rect2f unnormalize(const cv::Rect2f& r, int h, int w)
{
    auto unnorm = [](float x) -> float { return 0.5 * (x + 1); };
    auto tl = r.tl();
    auto br = r.br();
    auto x0 = unnorm(tl.x) * w;
    auto y0 = unnorm(tl.y) * h;
    auto x1 = unnorm(br.x) * w;
    auto y1 = unnorm(br.y) * h;
    return { x0, y0, x1 - x0, y1 - y0 };
}

// Returns width and height of the input tensor, or throws.
// Expects the model to take one tensor as input that must
// have the shape B x C x H x W, where B=C=1.
cv::Size get_input_image_shape(const Ort::Session& session)
{
    if (session.GetInputCount() < 1)
        throw std::invalid_argument("Model must take at least one input tensor");
    const std::vector<std::int64_t> shape = session.GetInputTypeInfo(0).GetTensorTypeAndShapeInfo().GetShape();
    if (shape.size() != 4)
        throw std::invalid_argument("Model takes the input tensor in the wrong shape");
    return { static_cast<int>(shape[3]), static_cast<int>(shape[2]) };
}

Ort::Value create_tensor(const Ort::TypeInfo& info, Ort::Allocator& alloc)
{
    const auto shape = info.GetTensorTypeAndShapeInfo().GetShape();
    auto t = Ort::Value::CreateTensor<float>(alloc, shape.data(), shape.size());
    memset(t.GetTensorMutableData<float>(), 0, sizeof(float) * info.GetTensorTypeAndShapeInfo().GetElementCount());
    return t;
}

int find_input_intensity_quantile(const cv::Mat& frame, float percentage)
{
    const int channels[] = { 0 };
    const int hist_size[] = { 256 };
    float range[] = { 0, 256 };
    const float* ranges[] = { range };
    cv::Mat hist;
    cv::calcHist(&frame, 1, channels, cv::Mat(), hist, 1, hist_size, ranges, true, false);
    int gray_level = 0;
    const int num_pixels_quantile = frame.total() * percentage * 0.01f;
    int num_pixels_accum = 0;
    for (int i = 0; i < hist_size[0]; ++i)
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

// Automatic brightness adjustment. Scales brightness to lie between -.5 and 0.5, roughly.
void normalize_brightness(const cv::Mat& frame, cv::Mat& out)
{
    const float pct = 90;

    const int brightness = find_input_intensity_quantile(frame, pct);

    const double alpha = brightness < 127 ? (pct / 100.f * 0.5f / std::max(5, brightness)) : 1. / 255;
    const double beta = -0.5;

    frame.convertTo(out, CV_32F, alpha, beta);
}

Localizer::Localizer(Ort::MemoryInfo& allocator_info, Ort::Session&& session)
    : session_{ std::move(session) }, scaled_frame_(INPUT_IMG_HEIGHT, INPUT_IMG_WIDTH, CV_8U), input_mat_(INPUT_IMG_HEIGHT, INPUT_IMG_WIDTH, CV_32F)
{
    // Only works when input_mat does not reallocated memory ...which it should not.
    // Non-owning memory reference to input_mat?
    // Note: shape = (bach x channels x h x w)
    const std::int64_t input_shape[4] = { 1, 1, INPUT_IMG_HEIGHT, INPUT_IMG_WIDTH };
    input_val_ = Ort::Value::CreateTensor<float>(allocator_info, input_mat_.ptr<float>(0), input_mat_.total(), input_shape, 4);

    const std::int64_t output_shape[2] = { 1, 5 };
    output_val_ = Ort::Value::CreateTensor<float>(allocator_info, results_.data(), results_.size(), output_shape, 2);
}

std::pair<float, cv::Rect2f> Localizer::run(const cv::Mat& frame)
{
    auto p = input_mat_.ptr(0);

    cv::resize(frame, scaled_frame_, { INPUT_IMG_WIDTH, INPUT_IMG_HEIGHT }, 0, 0, cv::INTER_AREA);
    scaled_frame_.convertTo(input_mat_, CV_32F, 1. / 255., -0.5);

    assert(input_mat_.ptr(0) == p);
    assert(!input_mat_.empty() && input_mat_.isContinuous());
    assert(input_mat_.cols == INPUT_IMG_WIDTH && input_mat_.rows == INPUT_IMG_HEIGHT);

    const char* input_names[] = { "x" };
    const char* output_names[] = { "logit_box" };

    Timer t;
    t.start();

    session_.Run(Ort::RunOptions{ nullptr }, input_names, &input_val_, 1, output_names, &output_val_, 1);

    last_inference_time_ = t.elapsed_ms();

    const cv::Rect2f roi = unnormalize(
        cv::Rect2f{
            results_[1], results_[2],
            results_[3] - results_[1], // Width
            results_[4] - results_[2]  // Height
        },
        frame.rows, frame.cols);
    const float score = sigmoid(results_[0]);

    return { score, roi };
}

double Localizer::last_inference_time_millis() const
{
    return last_inference_time_;
}

std::string PoseEstimator::get_network_input_name(size_t i) const
{
#if ORT_API_VERSION >= 12
    return std::string(&*session_.GetInputNameAllocated(i, allocator_));
#else
    return std::string(session_.GetInputName(i, allocator_));
#endif
}

std::string PoseEstimator::get_network_output_name(size_t i) const
{
#if ORT_API_VERSION >= 12
    return std::string(&*session_.GetOutputNameAllocated(i, allocator_));
#else
    return std::string(session_.GetOutputName(i, allocator_));
#endif
}

PoseEstimator::PoseEstimator(Ort::MemoryInfo& allocator_info, Ort::Session&& session)
    : model_version_{ session.GetModelMetadata().GetVersion() }, session_{ std::move(session) }, allocator_{ session_, allocator_info }
{
    using namespace std::literals::string_literals;

    if (session_.GetOutputCount() < 2)
        throw std::runtime_error("Invalid Model: must have at least two outputs");

    // WARNING: Messy model compatibility issues!
    // When reading the initial model release, it did not have the version field set.
    // Reading it here will result in some unspecified value. It's probably UB due to
    // reading uninitialized memory. But there is little choice.
    // Now, detection of this old version is messy ... we have to guess based on the
    // number we get. Getting an uninitialized value matching a valid version is unlikely.
    // But the real problem is that this line must be updated whenever we want to bump the
    // version number!!
    if (model_version_ <= 0 || model_version_ > 4)
        model_version_ = 1;

    const cv::Size input_image_shape = get_input_image_shape(session_);

    scaled_frame_ = cv::Mat(input_image_shape, CV_8U, cv::Scalar(0));
    input_mat_ = cv::Mat(input_image_shape, CV_32F, cv::Scalar(0.f));

    {
        const std::int64_t input_shape[4] = { 1, 1, input_image_shape.height, input_image_shape.width };
        input_val_.push_back(Ort::Value::CreateTensor<float>(allocator_info, input_mat_.ptr<float>(0), input_mat_.total(), input_shape, 4));
    }

    struct TensorSpec
    {
        std::vector<int64_t> shape;
        float* buffer = nullptr;
        size_t element_count = 0;
        bool available = false;
    };

    std::unordered_map<std::string, TensorSpec> understood_outputs = {
        { "pos_size", TensorSpec{ { 1, 3 }, &output_coord_[0], output_coord_.rows } },
        { "quat", TensorSpec{ { 1, 4 }, &output_quat_[0], output_quat_.rows } },
        { "box", TensorSpec{ { 1, 4 }, &output_box_[0], output_box_.rows } },
        { "rotaxis_scales_tril", TensorSpec{ { 1, 3, 3 }, output_rotaxis_scales_tril_.val, 9 } },
        { "rotaxis_std", TensorSpec{ { 1, 3, 3 }, output_rotaxis_scales_tril_.val, 9 } }, // TODO: Delete when old models aren't used any more
        { "pos_size_std", TensorSpec{ { 1, 3 }, output_coord_scales_std_.val, output_coord_scales_std_.rows } },
        { "pos_size_scales", TensorSpec{ { 1, 3 }, output_coord_scales_std_.val, output_coord_scales_std_.rows } },
        { "pos_size_scales_tril", TensorSpec{ { 1, 3, 3 }, output_coord_scales_tril_.val, 9 } }
    };

    qDebug() << "Pose model inputs (" << session_.GetInputCount() << ")";
    qDebug() << "Pose model outputs (" << session_.GetOutputCount() << "):";
    output_names_.resize(session_.GetOutputCount());
    output_c_names_.resize(session_.GetOutputCount());
    for (size_t i = 0; i < session_.GetOutputCount(); ++i)
    {
        const std::string name = get_network_output_name(i);
        const auto& output_info = session_.GetOutputTypeInfo(i);
        const auto& onnx_tensor_spec = output_info.GetTensorTypeAndShapeInfo();
        auto my_tensor_spec_it = understood_outputs.find(name);

        qDebug() << "\t" << name.c_str() << " (" << onnx_tensor_spec.GetShape() << ") dtype: " << onnx_tensor_spec.GetElementType() << " "
                 << (my_tensor_spec_it != understood_outputs.end() ? "ok" : "unknown");

        if (my_tensor_spec_it != understood_outputs.end())
        {
            TensorSpec& t = my_tensor_spec_it->second;
            if (onnx_tensor_spec.GetShape() != t.shape || onnx_tensor_spec.GetElementType() != Ort::TypeToTensorType<float>::type)
                throw std::runtime_error("Invalid output tensor spec for "s + name);
            output_val_.push_back(Ort::Value::CreateTensor<float>(allocator_info, t.buffer, t.element_count, t.shape.data(), t.shape.size()));
            t.available = true;
        }
        else
        {
            // Create tensor regardless and ignore output
            output_val_.push_back(create_tensor(output_info, allocator_));
        }
        output_names_[i] = name;
        output_c_names_[i] = output_names_[i].c_str();
    }

    has_uncertainty_ = understood_outputs.at("rotaxis_scales_tril").available || understood_outputs.at("rotaxis_std").available;
    has_uncertainty_ &= understood_outputs.at("pos_size_std").available || understood_outputs.at("pos_size_scales").available
                        || understood_outputs.at("pos_size_scales_tril").available;
    pos_scale_uncertainty_is_matrix_ = understood_outputs.at("pos_size_scales_tril").available;

    input_names_.resize(session_.GetInputCount());
    input_c_names_.resize(session_.GetInputCount());
    for (size_t i = 0; i < session_.GetInputCount(); ++i)
    {
        input_names_[i] = get_network_input_name(i);
        input_c_names_[i] = input_names_[i].c_str();
    }

    assert(input_names_.size() == input_val_.size());
    assert(output_names_.size() == output_val_.size());
}

std::optional<PoseEstimator::Face> PoseEstimator::run(const cv::Mat& frame, const cv::Rect& box)
{
    cv::Mat cropped;

    const int patch_size = std::max(box.width, box.height) * 1.05;
    const cv::Point2f patch_center = { std::clamp<float>(box.x + 0.5f * box.width, 0.f, frame.cols),
                                       std::clamp<float>(box.y + 0.5f * box.height, 0.f, frame.rows) };
    cv::getRectSubPix(frame, { patch_size, patch_size }, patch_center, cropped);

    // Will get failure if patch_center is outside image boundaries settings.
    // Have to catch this case.
    if (cropped.rows != patch_size || cropped.cols != patch_size)
        return {};

    [[maybe_unused]] auto* p = input_mat_.ptr(0);

    cv::resize(cropped, scaled_frame_, scaled_frame_.size(), 0, 0, cv::INTER_AREA);

    normalize_brightness(scaled_frame_, input_mat_);

    assert(input_mat_.ptr(0) == p);
    assert(!input_mat_.empty() && input_mat_.isContinuous());

    Timer t;
    t.start();

    try
    {
        session_.Run(Ort::RunOptions{ nullptr }, input_c_names_.data(), input_val_.data(), input_val_.size(), output_c_names_.data(),
                     output_val_.data(), output_val_.size());
    }
    catch (const Ort::Exception& e)
    {
        qDebug() << "Failed to run the model: " << e.what();
        return {};
    }

    last_inference_time_ = t.elapsed_ms();

    // Perform coordinate transformation.
    // From patch-local normalized in [-1,1] to
    // frame unnormalized pixel.

    cv::Matx33f center_size_cov_tril = {};
    if (has_uncertainty_)
    {
        if (pos_scale_uncertainty_is_matrix_)
        {
            center_size_cov_tril = output_coord_scales_tril_;
        }
        else
        {
            center_size_cov_tril(0, 0) = output_coord_scales_std_[0];
            center_size_cov_tril(1, 1) = output_coord_scales_std_[1];
            center_size_cov_tril(2, 2) = output_coord_scales_std_[2];
        }
        center_size_cov_tril *= patch_size * 0.5f;
    }

    const cv::Point2f center = patch_center + (0.5f * patch_size) * cv::Point2f{ output_coord_[0], output_coord_[1] };
    const float size = patch_size * 0.5f * output_coord_[2];

    // Following Eigen which uses quat components in the order w, x, y, z.
    // As does OpenCV
    cv::Quatf rotation = { output_quat_[3], output_quat_[0], output_quat_[1], output_quat_[2] };

    // Should be lower triangular. If not maybe something is wrong with memory layout ... or the model.
    assert(output_rotaxis_scales_tril_(0, 1) == 0);
    assert(output_rotaxis_scales_tril_(0, 2) == 0);
    assert(output_rotaxis_scales_tril_(1, 2) == 0);
    assert(center_size_cov_tril(0, 1) == 0);
    assert(center_size_cov_tril(0, 2) == 0);
    assert(center_size_cov_tril(1, 2) == 0);

    cv::Matx33f rotaxis_scales_tril = output_rotaxis_scales_tril_;

    if (model_version_ < 2)
    {
        // Due to a change in coordinate conventions
        rotation = world_to_image(rotation);
    }

    const cv::Rect2f outbox = { patch_center.x + (0.5f * patch_size) * output_box_[0], patch_center.y + (0.5f * patch_size) * output_box_[1],
                                0.5f * patch_size * (output_box_[2] - output_box_[0]), 0.5f * patch_size * (output_box_[3] - output_box_[1]) };

    return std::optional<Face>({ rotation, rotaxis_scales_tril, outbox, center, size, center_size_cov_tril });
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

} // namespace neuralnet_tracker_ns
