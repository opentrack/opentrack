/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core.hpp>
#include <vector>

//-----------------------------------------------------------------------------
// Calibrates the translation from head to model = t_MH
// by recursive least squares /
// kalman filter in information form with identity noise covariance
// measurement equation when head position = t_CH is fixed:
// (R_CM_k , Id)*(-t_MH, t_CH) = t_CM_k

class TranslationCalibrator
{
public:
    TranslationCalibrator(unsigned yaw_rdof, unsigned pitch_rdof);

    // reset the calibration process
    void reset();

    // update the current estimate
    void update(const cv::Matx33d& R_CM_k, const cv::Vec3d& t_CM_k);

    // get the current estimate for t_MH
    std::tuple<cv::Vec3f, unsigned> get_estimate();

private:
    bool check_bucket(const cv::Matx33d& R_CM_k);

    cv::Matx66f P;  // normalized precision matrix = inverse covariance
    cv::Vec6f y;    // P*(-t_MH, t_CH)

    // note, bin count's so small we don't need a bloom filter
    std::vector<bool> used_poses;

    static constexpr double yaw_spacing_in_degrees = 2.5;
    static constexpr double pitch_spacing_in_degrees = 1.5;

    // this allows allows us up to +-180 for yaw and pitch
    static constexpr int bin_count = 361*360 / (yaw_spacing_in_degrees*pitch_spacing_in_degrees) + 1;

    unsigned yaw_rdof, pitch_rdof, nsamples;
};
