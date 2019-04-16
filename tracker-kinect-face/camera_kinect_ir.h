/* Copyright (c) 2019, Stephane Lenclud <github@lenclud.com>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once

#ifdef OTR_HAVE_OPENCV

#include <Kinect.h>

//#include "pt-api.hpp"
#include "compat/timer.hpp"
#include "video/camera.hpp"


#include <memory>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

#include <QString>

namespace Kinect {

    struct CamerasProvider : video::impl::camera_
    {
        CamerasProvider();
        std::vector<QString> camera_names() const override;
        std::unique_ptr<video::impl::camera> make_camera(const QString& name) override;
        bool can_show_dialog(const QString& camera_name) override;
        bool show_dialog(const QString& camera_name) override;
    };


///
/// Implement our camera interface using Kinect V2 SDK IR Sensor.
///
struct InfraredCamera final : video::impl::camera
{
    InfraredCamera();
    ~InfraredCamera() override;

    // From video::impl::camera
    [[nodiscard]] bool start(info& args) override;
    void stop() override;
    bool is_open() override;
    std::tuple<const video::impl::frame&, bool> get_frame() override;
    [[nodiscard]] bool show_dialog() override;

private:
    bool get_frame_(cv::Mat& frame);
    void WaitForFirstFrame();

private:
    // Current Kinect
    IKinectSensor* iKinectSensor = nullptr;

    // Infrared reader
    IInfraredFrameReader*  iInfraredFrameReader = nullptr;

    // Frame needs to stay alive while we access the data buffer
    IInfraredFrame* iInfraredFrame = nullptr;

    //
    ICoordinateMapper* iCoordinateMapper = nullptr;

    video::frame iFrame;
    cv::Mat iMatFrame;

    float iFov = 0;
    int iWidth = 0, iHeight = 0;
    bool iFirstFrame = true;
};

}


#endif
