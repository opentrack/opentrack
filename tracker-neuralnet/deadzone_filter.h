#pragma once

#include "model_adapters.h"
#include "opencv_contrib.h"
#include "unscented_trafo.h"

namespace neuralnet_tracker_ns
{

/// Represents a 6d pose by quaternion rotation and position vector.
struct QuatPose
{
    cv::Quatf rot;
    cv::Vec3f pos;
};

struct FiltParams
{
    float deadzone_hardness = 1.f;
    float deadzone_size = 1.f;
};

/** Callback type for converting data from the `Face` struct to a 6d pose.
 *
 * This callback is needed because it depends on things that the filter doesn't have to know about and it is called multiple times
 * due to the way how uncertainty estimates are handled
 */
using Face2WorldFunction = std::function<QuatPose(const cv::Quatf&, const cv::Point2f&, float)>;

/** Applies a deadzone filter similar to the one used in the Hamilton filter.
 *
 * What sets this apart is that the deadzone size scales with the uncertainty estimate of the network.
 * The rotation uncertainty is represented by a covariance matrix for the distribution of a rotation vector which
 * describes the offset from the mean rotation (the quaternion in the `Face` struct).
 */
QuatPose apply_filter(const PoseEstimator::Face& face, const QuatPose& previous_pose, float dt, Face2WorldFunction face2world, const FiltParams& params);

} // namespace neuralnet_tracker_ns