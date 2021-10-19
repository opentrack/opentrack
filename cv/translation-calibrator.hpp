/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core/matx.hpp>
#include <vector>

//-----------------------------------------------------------------------------
// Calibrates the translation from head to model = t_MH
// by recursive least squares /
// kalman filter in information form with identity noise covariance
// measurement equation when head position = t_CH is fixed:
// (R_CM_k , Id)*(-t_MH, t_CH) = t_CM_k

class TranslationCalibrator
{
    bool check_bucket(const cv::Matx33d& R_CM_k);

    cv::Matx66f P;  // normalized precision matrix = inverse covariance
    cv::Vec6f y;    // P*(-t_MH, t_CH)

    using vec_i = std::vector<unsigned>;

    vec_i used_yaw_poses {};
    vec_i used_pitch_poses {};

    unsigned yaw_rdof, pitch_rdof, nsamples = 0;

public:
    TranslationCalibrator(unsigned yaw_rdof, unsigned pitch_rdof);
    void reset();

    // update the current estimate
    void update(const cv::Matx33d& R_CM_k, const cv::Vec3d& t_CM_k);

    // we're bringing in 3DOF samples but the calibrator only
    // checks yaw and pitch

    static constexpr unsigned num_cal_axis = 3;
    static constexpr unsigned num_nsample_axis = 2;

    using cv_cal_vec = cv::Vec<float, num_cal_axis>;
    using cv_nsample_vec = cv::Vec<unsigned, num_nsample_axis>;
    using tt = std::tuple<cv_cal_vec, cv_nsample_vec>;

    // get the current estimate for t_MH
    tt get_estimate();

    static constexpr double yaw_spacing_in_degrees = 2;
    static constexpr double pitch_spacing_in_degrees = 1.5;
};
