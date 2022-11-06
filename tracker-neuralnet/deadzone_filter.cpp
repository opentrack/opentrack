#include "deadzone_filter.h"
#include "model_adapters.h"
#include "opencv_contrib.h"
#include "unscented_trafo.h"

#include <opencv2/core/base.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core/quaternion.hpp>

namespace neuralnet_tracker_ns
{

using namespace cvcontrib;

using StateVec = cv::Vec<float,6>;
using StateCov = cv::Matx<float,6,6>;

static constexpr int num_sigmas = ukf_cv::MerweScaledSigmaPoints<6>::num_sigmas;
static constexpr float img_scale = 200.f;
static constexpr float world_scale = 1000.f; // mm

StateCov make_tangent_space_uncertainty_tril(const PoseEstimator::Face &face)
{
    StateCov tril = StateCov::eye();
    tril(0,0) = face.center_stddev.x/img_scale;
    tril(1,1) = face.center_stddev.y/img_scale;
    tril(2,2) = face.size_stddev/img_scale;
    set_minor<3,3>(tril, 3, 3, face.rotaxis_cov_tril);
    return tril;
}


QuatPose apply_offset(const QuatPose& pose, const StateVec& offset)
{
    // Unpack
    const cv::Vec3f dp = { offset[0], offset[1], offset[2] };
    const cv::Quatf dr = cv::Quatf::createFromRvec(cv::Vec3f{ offset[3], offset[4], offset[5] });
    const auto p = pose.pos + dp;
    const auto r = pose.rot * dr;
    return { r, p };
}


PoseEstimator::Face apply_offset(const PoseEstimator::Face& face, const StateVec& offset)
{
    const cv::Quatf dr = cv::Quatf::createFromRvec(cv::Vec3f{ offset[3], offset[4], offset[5] });
    const auto r = face.rotation * dr;
    
    const cv::Point2f p = { 
        face.center.x + offset[0]*img_scale, 
        face.center.y + offset[1]*img_scale 
    };

    // Intercept the case where the head size stddev became so large that the sigma points
    // were created with negative head size (mean - constant*stddev ...). Negative head size
    // is bad. But this is fine. The unscented transform where this function comes into play
    // is designed to handle non-linearities like this.
    const float sz = std::max(0.1f*face.size, face.size + offset[2]*img_scale);

    return PoseEstimator::Face{
        r,
        {},
        {},
        p,
        {},
        sz,
        {}
    };
}


StateVec relative_to(const QuatPose& reference, const QuatPose& pose)
{
    const auto p = pose.pos - reference.pos;
    const auto r = toRotVec(reference.rot.conjugate()*pose.rot);
    return StateVec{ p[0], p[1], p[2], r[0], r[1], r[2] };
}


ukf_cv::SigmaPoints<6> relative_to(const QuatPose& pose, const std::array<QuatPose,num_sigmas>& sigmas)
{
    ukf_cv::SigmaPoints<6> out;
    std::transform(sigmas.begin(), sigmas.end(), out.begin(), [&pose](const QuatPose& s) {
        return relative_to(pose, s);
    });
    return out;
}


std::array<QuatPose,num_sigmas> compute_world_pose_from_sigma_point(const PoseEstimator::Face& face, const ukf_cv::SigmaPoints<6>& sigmas, Face2WorldFunction face2world)
{
    std::array<QuatPose,num_sigmas> out;
    std::transform(sigmas.begin(), sigmas.end(), out.begin(), [face2world=std::move(face2world), &face](const StateVec& sigma_point) {
        // First unpack the state vector and generate quaternion rotation w.r.t image space.
        const auto sigma_face = apply_offset(face, sigma_point);
        // Then transform ...
        QuatPose pose = face2world(sigma_face.rotation, sigma_face.center, sigma_face.size);
        pose.pos /= world_scale;
        return pose;
    });
    return out;
}


StateVec apply_filter_to_offset(const StateVec& offset, const StateCov& offset_cov, float, const FiltParams& params)
{
    // Offset and Cov represent a multivariate normal distribution, which is the probability of the new pose measured w.r.t the previous one.
    // Prob(x) ~exp(-(x-mu)t Cov^-1 (x-mu))
    // We want to attenuate this offset, or zero it out completely, to obtain a deadzone-filter behaviour. The size of the deadzone shall be
    // determined by the covariance projected to the offset direction like so:
    // Take x = mu - mu / |mu| * alpha
    // p(alpha) ~exp(-alpha^2 / |mu|^2 * mut Cov^-1 mu) = ~exp(-alpha^2 / sigma^2) with sigma^2 = mut Cov^-1 mu / |mu|^2.
    // So this projection is like a 1d normal distribution with some standard deviation, which we take to scale the deadzone.

    bool ok = true;

    const float len_div_sigma_sqr = offset.dot(offset_cov.inv(cv::DECOMP_CHOLESKY, &ok) * offset);
    
    const float attenuation = (ok) ? sigmoid((std::sqrt(len_div_sigma_sqr) - params.deadzone_size)*params.deadzone_hardness) : 1.f;

    // {
    //     std::cout << "cov diag: " << offset_cov.diag() << std::endl;
    //     std::cout << "offset: " << cv::norm(offset) << std::endl;
    //     std::cout << "len_div_sigma_sqr: " << cv::norm(len_div_sigma_sqr) << std::endl;
    //      std::cout << "attenuation (" << ok << "): " << attenuation << std::endl;
    // }

    return offset*attenuation;
}


QuatPose apply_filter(const PoseEstimator::Face &face, const QuatPose& previous_pose_, float dt, Face2WorldFunction face2world, const FiltParams& params)
{
    ukf_cv::MerweScaledSigmaPoints<6> unscentedtrafo;
    auto previous_pose = previous_pose_;
    previous_pose.pos /= world_scale;

    // Here we have the covariance matrix for the offset from the observed values in `face`.
    const auto cov_tril = make_tangent_space_uncertainty_tril(face);

    // The filter uses an unscented transform to translate that into a distribution for the offset from the previous pose.
    const ukf_cv::SigmaPoints<6> sigmas = unscentedtrafo.compute_sigmas(to_vec(StateVec::zeros()), cov_tril, true);

    // We have many of these sigma points. This is why that callback comes into play here.
    const std::array<QuatPose,num_sigmas> pose_sigmas = compute_world_pose_from_sigma_point(face, sigmas, std::move(face2world));

    const ukf_cv::SigmaPoints<6> deltas_sigmas = relative_to(previous_pose, pose_sigmas);

    const auto [offset, offset_cov] = unscentedtrafo.compute_statistics(deltas_sigmas);

    // Then the deadzone is applied to the offset and finally the previous pose is transformed by the offset to arrive at the final output.
    const StateVec scaled_offset = apply_filter_to_offset(offset, offset_cov, dt, params);

    QuatPose new_pose = apply_offset(previous_pose, scaled_offset);

    new_pose.pos *= world_scale;

    return new_pose;
}


} // namespace neuralnet_tracker_ns