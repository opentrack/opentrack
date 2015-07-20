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
bool get_camera_calibration(const QString& camera_name, cv::Mat& intrinsics, cv::Mat& distortion, int w, int h, int fov)
{
    const QString pathnames[] = {
        QCoreApplication::applicationDirPath() + "/camera/" + camera_name + "-" + QString::number(fov) + ".yml",
        QCoreApplication::applicationDirPath() + "/camera/" + camera_name + ".yml",
    };
    for (auto& pathname : pathnames)
    {
        cv::FileStorage fs(pathname.toStdString(), cv::FileStorage::READ);
        if (!fs.isOpened())
            continue;
        cv::Mat intrinsics_, distortion_;
        fs["camera_matrix"] >> intrinsics_;
        fs["distortion_coefficients"] >> distortion_;
        int w_, h_;
        fs["image_width"] >> w_;
        fs["image_height"] >> h_;
        double w__ = w_, h__ = h_;
        intrinsics_.at<float>(0, 0) *= w / w__;
        intrinsics_.at<float>(2, 0) *= w / w__;
        intrinsics_.at<float>(1, 1) *= h / h__;
        intrinsics_.at<float>(2, 1) *= h / h__;
        intrinsics = intrinsics_;
        distortion = distortion_;
        return true;
    }
    return false;
}
