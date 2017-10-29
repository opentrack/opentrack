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
constexpr double TranslationCalibrator::roll_spacing_in_degrees;

TranslationCalibrator::TranslationCalibrator(unsigned yaw_rdof, unsigned pitch_rdof, unsigned roll_rdof) :
    yaw_rdof(yaw_rdof), pitch_rdof(pitch_rdof), roll_rdof(roll_rdof)
{
    reset();
}

void TranslationCalibrator::reset()
{
    P = cv::Matx66f::zeros();
    y = cv::Vec6f(0,0,0, 0,0,0);

    used_yaw_poses = vec(1 + iround(360 / yaw_spacing_in_degrees), 0);
    used_pitch_poses = vec(1 + iround(360 / pitch_spacing_in_degrees), 0);
    used_roll_poses = vec(1 + iround(360 / roll_spacing_in_degrees), 0);

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

std::tuple<cv::Vec3f, cv::Vec3i> TranslationCalibrator::get_estimate()
{
    cv::Vec6f x = P.inv() * y;

    unsigned values[3] {};
    vec* in[] { &used_yaw_poses, &used_pitch_poses, &used_roll_poses };

    for (unsigned k = 0; k < 3; k++)
    {
        const vec& data = *in[k];
        for (unsigned i = 0; i < data.size(); i++)
            if (data[i])
                values[k]++;
    }

    qDebug() << "samples total" << nsamples
             << "yaw" << values[0]
             << "pitch" << values[1]
             << "roll" << values[2];

    return std::make_tuple(cv::Vec3f(-x[0], -x[1], -x[2]),
                           cv::Vec3i(values[0], values[1], values[2]));
}

bool TranslationCalibrator::check_bucket(const cv::Matx33d& R_CM_k)
{
    using namespace euler;
    constexpr double r2d = 180/M_PI;

    rmat r;
    for (unsigned j = 0; j < 3; j++)
        for (unsigned i = 0; i < 3; i++)
            r(j, i) = R_CM_k(j, i);

    const euler_t ypr = rmat_to_euler(r) * r2d;

    const unsigned yaw_k = iround((ypr(yaw_rdof) + 180)/yaw_spacing_in_degrees);
    const unsigned pitch_k = iround((ypr(pitch_rdof) + 180)/pitch_spacing_in_degrees);
    const unsigned roll_k = iround((ypr(roll_rdof) + 180)/roll_spacing_in_degrees);

    if (yaw_k < used_yaw_poses.size() &&
        pitch_k < used_pitch_poses.size() &&
        roll_k < used_roll_poses.size())
    {
        used_yaw_poses[yaw_k]++;
        used_pitch_poses[pitch_k]++;
        used_roll_poses[roll_k]++;

        return used_yaw_poses[yaw_k] == 1 ||
               used_pitch_poses[pitch_k] == 1 ||
               used_roll_poses[roll_k] == 1;
    }
    else
        qDebug() << "calibrator: index out of range"
                 << "yaw" << yaw_k
                 << "pitch" << pitch_k
                 << "roll" << roll_k;

    return false;
}
