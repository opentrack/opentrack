#pragma once

#include <algorithm>
#include <opencv2/core.hpp>
#include <opencv2/core/base.hpp>
#include <opencv2/core/quaternion.hpp>

#include <cmath>
#include <vector>

#include "opencv_contrib.h"

namespace ukf_cv
{

using namespace cvcontrib;

template <int dim, int otherdim = dim> using SigmaPoints = std::array<cv::Vec<float, otherdim>, dim * 2 + 1>;

// Ported from
// https://filterpy.readthedocs.io/en/latest/_modules/filterpy/kalman/sigma_points.html
// Excerpt from the original docu:
// "

// Generates sigma points and weights according to Van der Merwe's
// 2004 dissertation[1] for the UnscentedKalmanFilter class.. It
// parametizes the sigma points using alpha, beta, kappa terms, and
// is the version seen in most publications.

// Unless you know better, this should be your default choice.

// alpha : float
//     Determins the spread of the sigma points around the mean.
//     Usually a small positive value (1e-3) according to [3].

// beta : float
//     Incorporates prior knowledge of the distribution of the mean. For
//     Gaussian x beta=2 is optimal, according to [3].

// kappa : float, default=0.0
//     Secondary scaling parameter usually set to 0 according to [4],
//     or to 3-n according to [5].

// Reference
// .. [1] R. Van der Merwe "Sigma-Point Kalman Filters for Probabilitic
//        Inference in Dynamic State-Space Models" (Doctoral dissertation)

// "
template <int dim> class MerweScaledSigmaPoints
{
public:
    static constexpr int num_sigmas = 2 * dim + 1;

    using Vector = cv::Vec<float, dim>;
    using Matrix = cv::Matx<float, dim, dim>;

    MerweScaledSigmaPoints(float alpha = 0.01, float beta = 2., int kappa = 3 - dim)
    {
        lambda = alpha * alpha * (dim + kappa) - dim;
        const float c = .5 / (dim + lambda);
        Wc_i = c;
        Wm_i = c;
        Wm_0 = lambda / (dim + lambda);
        Wc_0 = Wm_0 + (1. - alpha * alpha + beta);
    }

    SigmaPoints<dim> compute_sigmas(const Vector& mu, const Matrix& mat, bool is_tril_factor) const
    {
        const Matrix triu_factor = is_tril_factor ? mat.t() : cholesky(mat).t();

        const Matrix U = triu_factor * std::sqrt(lambda + dim);

        SigmaPoints<dim> sigmas;

        sigmas[0] = mu;
        for (int k = 0; k < dim; ++k)
        {
            sigmas[k + 1] = to_vec(mu + U.row(k).t());
            sigmas[dim + k + 1] = to_vec(mu - U.row(k).t());
        }
        return sigmas;
    }

    template <int otherdim>
    std::tuple<cv::Vec<float, otherdim>, cv::Matx<float, otherdim, otherdim>> compute_statistics(const SigmaPoints<dim, otherdim>& sigmas) const
    {
        cv::Vec<float, otherdim> mu{}; // Zero initializes
        for (size_t i = 0; i < sigmas.size(); ++i)
        {
            mu += to_vec((i == 0 ? Wm_0 : Wm_i) * sigmas[i]);
        }

        cv::Matx<float, otherdim, otherdim> cov{};
        for (size_t i = 0; i < sigmas.size(); ++i)
        {
            const auto p = sigmas[i] - mu;
            cov += (i == 0 ? Wc_0 : Wc_i) * p * p.t();
        }

        return { mu, cov };
    }

    template <int otherdim>
    cv::Matx<float, dim, otherdim> compute_cov(const SigmaPoints<dim, dim>& sigmas, const SigmaPoints<dim, otherdim>& othersigmas) const
    {
        cv::Vec<float, dim> mu{};            // Zero initializes
        cv::Vec<float, otherdim> mu_other{}; // Zero initializes
        for (size_t i = 0; i < sigmas.size(); ++i)
        {
            mu += to_vec((i == 0 ? Wm_0 : Wm_i) * sigmas[i]);
            mu_other += to_vec((i == 0 ? Wm_0 : Wm_i) * othersigmas[i]);
        }

        cv::Matx<float, dim, otherdim> cov{};
        for (size_t i = 0; i < sigmas.size(); ++i)
        {
            const auto p = sigmas[i] - mu;
            const auto q = othersigmas[i] - mu_other;
            cov += (i == 0 ? Wc_0 : Wc_i) * p * q.t();
        }

        return cov;
    }

private:
    float Wc_i, Wm_i, Wm_0, Wc_0, lambda;
};

} // namespace ukf_cv