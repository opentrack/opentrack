/* Copyright (c) 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "export.hpp"

#include <memory>
#include <vector>

#include <QString>

namespace video
{

struct frame final
{
    unsigned char* data = nullptr;
    // the `stride' member can have a special value of zero,
    // signifying equal to width * element size
    int width = 0, height = 0, stride = 0, channels = 0;
    int channelSize = 1;
};

} // ns video

namespace video::impl {

using namespace video;

struct camera;

struct OTR_VIDEO_EXPORT camera_
{
    camera_();
    virtual ~camera_();

    virtual std::vector<QString> camera_names() const = 0;
    virtual std::unique_ptr<camera> make_camera(const QString& name) = 0;
    [[nodiscard]] virtual bool show_dialog(const QString& camera_name) = 0;
    virtual bool can_show_dialog(const QString& camera_name) = 0;
};

struct OTR_VIDEO_EXPORT camera
{
    struct info final
    {
        int width = 0, height = 0, fps = 0;
        float focalLengthX = 0.0f;
        float focalLengthY = 0.0f;
        float principalPointX = 0.0f;
        float principalPointY = 0.0f;
        float radialDistortionSecondOrder = 0.0f;
        float radialDistortionFourthOrder = 0.0f;
        float radialDistortionSixthOrder = 0.0f;
    };

    camera();
    virtual ~camera();

    [[nodiscard]] virtual bool start(info& args) = 0;
    virtual void stop() = 0;
    virtual bool is_open() = 0;

    virtual std::tuple<const frame&, bool> get_frame() = 0;
    [[nodiscard]] virtual bool show_dialog() = 0;
};

OTR_VIDEO_EXPORT
void register_camera(std::unique_ptr<impl::camera_> metadata);

} // ns video::impl

#define OTR_REGISTER_CAMERA3(type, ctr)                                 \
    static const char init_ ## ctr =                                    \
        (::video::impl::register_camera(std::make_unique<type>()), 0);

#define OTR_REGISTER_CAMERA2(type, ctr) \
    OTR_REGISTER_CAMERA3(type, ctr)

#define OTR_REGISTER_CAMERA(type) \
    OTR_REGISTER_CAMERA2(type, __COUNTER__)

namespace video
{
using camera_impl = typename impl::camera;

OTR_VIDEO_EXPORT
std::unique_ptr<camera_impl> make_camera(const QString& name);

OTR_VIDEO_EXPORT
std::vector<QString> camera_names();

[[nodiscard]]
OTR_VIDEO_EXPORT
bool show_dialog(const QString& camera_name);

} // ns video
