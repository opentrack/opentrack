/* Copyright (c) 2019 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "compat/sleep.hpp"
#include "video/camera.hpp"

#include "camera-names.hpp"
#include "video-property-page.hpp"

#include <optional>

#include <opencv2/core.hpp>
#include <opencv2/videoio.hpp>

using namespace video::impl;

struct cam;

struct metadata : camera_
{
    metadata();
    std::vector<QString> camera_names() const override;
    std::unique_ptr<camera> make_camera(const QString& name) override;
    bool can_show_dialog(const QString& camera_name) override;
    bool show_dialog(const QString& camera_name) override;
};

struct cam final : camera
{
    cam(int idx);
    ~cam() override;

    bool start(const info& args) override;
    void stop() override;
    bool is_open() override;
    std::tuple<const frame&, bool> get_frame() override;
    bool show_dialog() override;

    bool get_frame_();

    std::optional<cv::VideoCapture> cap;
    cv::Mat mat;
    frame frame_;
    int idx = -1;
};

metadata::metadata() = default;

std::unique_ptr<camera> metadata::make_camera(const QString& name)
{
    int idx = camera_name_to_index(name);
    if (idx != -1)
        return std::make_unique<cam>(idx);
    else
        return nullptr;
}

std::vector<QString> metadata::camera_names() const
{
    return get_camera_names();
}

bool metadata::can_show_dialog(const QString& camera_name)
{
    return camera_name_to_index(camera_name) != -1;
}

bool metadata::show_dialog(const QString& camera_name)
{
    int idx = camera_name_to_index(camera_name);
    if (idx != -1)
    {
        video_property_page::show(idx);
        return true;
    }
    else
        return false;
}

cam::cam(int idx) : idx(idx)
{
}

cam::~cam()
{
    stop();
}

void cam::stop()
{
    if (cap)
    {
        if (cap->isOpened())
            cap->release();
        cap = std::nullopt;
    }
    mat = cv::Mat();
    frame_ = { {}, false };
}

bool cam::is_open()
{
    return !!cap;
}

bool cam::start(const info& args)
{
    stop();
    cap.emplace(idx);

    if (args.width > 0 && args.height > 0)
    {
        cap->set(cv::CAP_PROP_FRAME_WIDTH,  args.width);
        cap->set(cv::CAP_PROP_FRAME_HEIGHT, args.height);
    }
    if (args.fps > 0)
        cap->set(cv::CAP_PROP_FPS, args.fps);

    if (!cap->isOpened())
        goto fail;

    if (!get_frame_())
        goto fail;

    return true;

fail:
    stop();
    return false;
}

bool cam::get_frame_()
{
    if (!is_open())
        return false;

    for (unsigned i = 0; i < 10; i++)
    {
        if (cap->read(mat))
        {
            frame_.data = mat.data;
            frame_.width = mat.cols;
            frame_.height = mat.rows;
            frame_.stride = mat.step.p[0];
            frame_.channels = mat.channels();

            return true;
        }
        portable::sleep(50);
    }

    return false;
}

std::tuple<const frame&, bool> cam::get_frame()
{
    bool ret = get_frame_();
    return { frame_, ret };
}

bool cam::show_dialog()
{
    if (is_open())
        return video_property_page::show_from_capture(*cap, idx);
    else
        return video_property_page::show(idx);
}

OTR_REGISTER_CAMERA(metadata)
