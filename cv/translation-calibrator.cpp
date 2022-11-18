/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "translation-calibrator.hpp"
#include "compat/euler.hpp"
#include "compat/math.hpp"
#include "compat/macros.h"
#include <opencv2/core.hpp>

#include <tuple>

#include <QDebug>

TranslationCalibrator::TranslationCalibrator(unsigned yaw_rdof, unsigned pitch_rdof) :
    yaw_rdof(yaw_rdof), pitch_rdof(pitch_rdof)
{
    reset();
}

void TranslationCalibrator::reset()
{
    P = cv::Matx66f::zeros();
    y = { 0,0,0, 0,0,0 };

    used_yaw_poses = vec_i(1 + iround(360 / yaw_spacing_in_degrees), 0);
    used_pitch_poses = vec_i(1 + iround(360 / pitch_spacing_in_degrees), 0);

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

using cv_out_vec = TranslationCalibrator::cv_nsample_vec;
using cv_in_vec = TranslationCalibrator::cv_cal_vec;
using tt = TranslationCalibrator::tt;

tt TranslationCalibrator::get_estimate()
{
    cv::Vec6f x = P.inv() * y;

    vec_i const* in[num_nsample_axis] = { &used_yaw_poses, &used_pitch_poses, };
    unsigned nsamples[num_cal_axis] {};

    for (unsigned k = 0; k < num_nsample_axis; k++)
    {
        vec_i const& data = *in[k];
        for (unsigned i : data)
            if (i)
                nsamples[k]++;
    }

    qDebug() << "samples total" << nsamples
             << "yaw" << nsamples[0]
             << "pitch" << nsamples[1];

    return {
        { -x[0], -x[1], -x[2] },
        { nsamples[0], nsamples[1] },
    };
}

static constexpr double r2d = 180/M_PI;

bool TranslationCalibrator::check_bucket(const cv::Matx33d& R_CM_k)
{
    using namespace euler;

    rmat r;
    for (unsigned j = 0; j < 3; j++)
        for (unsigned i = 0; i < 3; i++)
            r(j, i) = R_CM_k(j, i);

    const Pose_ ypr = rmat_to_euler(r) * r2d;

    auto array_index = [](double val, double spacing) {
        return (unsigned)iround((val + 180)/spacing);
    };

    const unsigned yaw_k = array_index(ypr(yaw_rdof), yaw_spacing_in_degrees);
    const unsigned pitch_k = array_index(ypr(pitch_rdof), pitch_spacing_in_degrees);

    if (yaw_k < used_yaw_poses.size() &&
        pitch_k < used_pitch_poses.size())
    {
        used_yaw_poses[yaw_k]++;
        used_pitch_poses[pitch_k]++;

        return used_yaw_poses[yaw_k] == 1 ||
               used_pitch_poses[pitch_k] == 1;
    }
    else
    {
        eval_once(qDebug() << "calibrator: index out of range"
                           << "yaw" << yaw_k
                           << "pitch" << pitch_k);

        return false;
    }
}
