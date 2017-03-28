/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "translation-calibrator.hpp"
#include "compat/euler.hpp"
#include "compat/util.hpp"

#include <tuple>

constexpr double TranslationCalibrator::pitch_spacing_in_degrees;
constexpr double TranslationCalibrator::yaw_spacing_in_degrees;

TranslationCalibrator::TranslationCalibrator(unsigned yaw_rdof, unsigned pitch_rdof) :
    yaw_rdof(yaw_rdof), pitch_rdof(pitch_rdof)
{
    reset();
}

void TranslationCalibrator::reset()
{
    P = cv::Matx66f::zeros();
    y = cv::Vec6f(0,0,0, 0,0,0);

    used_poses = std::vector<bool>(bin_count, false);
    nsamples = 0;
}

void TranslationCalibrator::update(const cv::Matx33d& R_CM_k, const cv::Vec3d& t_CM_k)
{
    if (!check_bucket(R_CM_k))
        return;

    nsamples++;

    cv::Matx<double, 6,3> H_k_T = cv::Matx<double, 6,3>::zeros();
    for (int i=0; i<3; ++i) {
        for (int j=0; j<3; ++j) {
            H_k_T(i,j) = R_CM_k(j,i);
        }
    }
    for (int i=0; i<3; ++i)
    {
        H_k_T(3+i,i) = 1.0;
    }
    P += H_k_T * H_k_T.t();
    y += H_k_T * t_CM_k;
}

std::tuple<cv::Vec3f, unsigned> TranslationCalibrator::get_estimate()
{
    cv::Vec6f x = P.inv() * y;

    qDebug() << "calibrator:" << nsamples << "samples total";

    return std::make_tuple(cv::Vec3f(-x[0], -x[1], -x[2]), nsamples);
}

bool TranslationCalibrator::check_bucket(const cv::Matx33d& R_CM_k)
{
    using namespace euler;
    static constexpr double r2d = 180/M_PI;

    rmat r;
    for (unsigned j = 0; j < 3; j++)
        for (unsigned i = 0; i < 3; i++)
            r(j, i) = R_CM_k(j, i);

    const euler_t ypr = rmat_to_euler(r) * r2d;

    const int yaw = iround(ypr(yaw_rdof) + 180/yaw_spacing_in_degrees);
    const int pitch = iround(ypr(pitch_rdof) + 180/pitch_spacing_in_degrees);
    const int idx = pitch * 360/pitch_spacing_in_degrees + yaw;

    if (idx >= 0 && idx < bin_count)
    {
        if (used_poses[idx])
        {
            return false;
        }
        else
        {
            used_poses[idx] = true;
            return true;
        }
    }
    else
        qDebug() << "calibrator: index out of range" << "yaw" << yaw << "pitch" << pitch << "max" << bin_count;

    return false;
}
