#pragma once

#include <array>
#include <optional>
#include <string>
#include <vector>

#include "opencv_contrib.h"
#include <onnxruntime_cxx_api.h>
#include <opencv2/core.hpp>

namespace neuralnet_tracker_ns
{

// Generally useful sigmoid function
float sigmoid(float x);

class Localizer
{
public:
    Localizer(Ort::MemoryInfo& allocator_info, Ort::Session&& session);

    // Returns bounding wrt image coordinate of the input image
    // The preceeding float is the score for being a face normalized to [0,1].
    std::pair<float, cv::Rect2f> run(const cv::Mat& frame);

    double last_inference_time_millis() const;

private:
    inline static constexpr int INPUT_IMG_WIDTH = 288;
    inline static constexpr int INPUT_IMG_HEIGHT = 224;
    Ort::Session session_{ nullptr };
    // Inputs / outputs
    cv::Mat scaled_frame_{}, input_mat_{};
    Ort::Value input_val_{ nullptr }, output_val_{ nullptr };
    std::array<float, 5> results_;
    double last_inference_time_ = 0;
};

class PoseEstimator
{
public:
    struct Face
    {
        cv::Quatf rotation;
        cv::Matx33f rotaxis_cov_tril; // Lower triangular factor of Cholesky decomposition
        cv::Rect2f box;
        cv::Point2f center;
        float size;
        cv::Matx33f center_size_cov_tril; // Lower triangular factor of Cholesky decomposition
    };

    PoseEstimator(Ort::MemoryInfo& allocator_info, Ort::Session&& session);
    /** Inference
     *
     * Coordinates are defined wrt. the image space of the input `frame`.
     * X goes right, Z (depth) into the image, Y points down (like pixel coordinates values increase from top to bottom)
     */
    std::optional<Face> run(const cv::Mat& frame, const cv::Rect& box);
    // Returns an image compatible with the 'frame' image for displaying.
    cv::Mat last_network_input() const;
    double last_inference_time_millis() const;
    bool has_uncertainty() const { return has_uncertainty_; }

private:
    std::string get_network_input_name(size_t i) const;
    std::string get_network_output_name(size_t i) const;
    int64_t model_version_ = 0;        // Queried meta data from the ONNX file
    Ort::Session session_{ nullptr };  // ONNX's runtime context for running the model
    mutable Ort::Allocator allocator_; // Memory allocator for tensors
    // Inputs
    cv::Mat scaled_frame_{}, input_mat_{};   // Input. One is the original crop, the other is rescaled (?)
    std::vector<Ort::Value> input_val_;      // Tensors to put into the model
    std::vector<std::string> input_names_;   // Refers to the names in the onnx model.
    std::vector<const char*> input_c_names_; // Refers to the C names in the onnx model.
    // Outputs
    cv::Vec<float, 3> output_coord_{}; // 2d Coordinate and head size output.
    cv::Vec<float, 4> output_quat_{};  //  Quaternion output
    cv::Vec<float, 4> output_box_{};   // Bounding box output
    cv::Matx33f output_rotaxis_scales_tril_{}; // Lower triangular matrix of LLT factorization of covariance of rotation vector as offset from output quaternion
    cv::Matx33f output_coord_scales_tril_{};  // Lower triangular factor
    cv::Vec3f output_coord_scales_std_{};     // Depending on the model, alternatively a 3d vector with standard deviations.
    std::vector<Ort::Value> output_val_;      // Tensors to put the model outputs in.
    std::vector<std::string> output_names_;   // Refers to the names in the onnx model.
    std::vector<const char*> output_c_names_; // Refers to the C names in the onnx model.
    // More bookkeeping
    double last_inference_time_ = 0;
    bool has_uncertainty_ = false;
    bool pos_scale_uncertainty_is_matrix_ = false;
};

// Finds the intensity where x percent of pixels have less intensity than that.
int find_input_intensity_quantile(const cv::Mat& frame, float percentage);

// Adjust brightness levels to full range and scales the value range to [-0.5, 0.5]
void normalize_brightness(const cv::Mat& frame, cv::Mat& out);

} // namespace neuralnet_tracker_ns
