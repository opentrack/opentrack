/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "translation-calibrator.hpp"

TranslationCalibrator::TranslationCalibrator()
{
    reset();
}

void TranslationCalibrator::reset()
{
    P = cv::Matx66f::zeros();
    y = cv::Vec6f(0,0,0, 0,0,0);
}

void TranslationCalibrator::update(const cv::Matx33d& R_CM_k, const cv::Vec3d& t_CM_k)
{
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

cv::Vec3f TranslationCalibrator::get_estimate()
{
    cv::Vec6f x = P.inv() * y;
    return cv::Vec3f(-x[0], -x[1], -x[2]);
}
