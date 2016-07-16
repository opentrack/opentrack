/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include <opencv2/core/core.hpp>

//-----------------------------------------------------------------------------
// Calibrates the translation from head to model = t_MH
// by recursive least squares /
// kalman filter in information form with identity noise covariance
// measurement equation when head position = t_CH is fixed:
// (R_CM_k , Id)*(-t_MH, t_CH) = t_CM_k

class TranslationCalibrator
{
public:
    TranslationCalibrator();

    // reset the calibration process
    void reset();

    // update the current estimate
    void update(const cv::Matx33d& R_CM_k, const cv::Vec3d& t_CM_k);

    // get the current estimate for t_MH
    cv::Vec3f get_estimate();

private:
    cv::Matx66f P;  // normalized precision matrix = inverse covariance
    cv::Vec6f y;    // P*(-t_MH, t_CH)
};
