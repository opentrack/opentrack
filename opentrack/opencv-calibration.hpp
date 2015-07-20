/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include <QCoreApplication>
#include <QString>
#include <QByteArray>
#include <string>
#include <opencv2/core.hpp>

template<typename = void>
bool get_camera_calibration(const QString& camera_name, cv::Mat& intrinsics, cv::Mat& distortion, int w, int h)
{
    QString pathname_ = QCoreApplication::applicationDirPath() + "/camera/" + camera_name + ".yml";
    std::string pathname = pathname_.toStdString();
    cv::FileStorage fs(pathname, cv::FileStorage::READ);
    if (!fs.isOpened())
        return false;
    cv::Mat intrinsics_, distortion_;
    fs["camera_matrix"] >> intrinsics_;
    fs["distortion_coefficients"] >> distortion_;
    intrinsics_.at<double>(0, 0) *= w / 640.;
    intrinsics_.at<double>(2, 0) *= w / 640.;
    intrinsics_.at<double>(1, 1) *= h / 480.;
    intrinsics_.at<double>(2, 1) *= h / 480.;
    intrinsics = intrinsics_;
    distortion = distortion_;
    return true;
}
