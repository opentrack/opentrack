#pragma once

#include "video/camera.hpp"
#include "shm-layout.hpp"
#include "compat/shm.h"
#include "options/options.hpp"
#include "compat/macros1.h"

using namespace options;

#include <QProcess>

using video::impl::camera;
using video::impl::camera_;
using video::frame;

struct settings : opts
{
    settings() : opts{"video-ps3eye"} {}

    value<slider_value> exposure{b, "exposure", {63, 0, 63}};
    value<slider_value> gain{b, "gain", {30, 0, 63}};
};

struct ps3eye_camera final : video::impl::camera
{
    QProcess wrapper;
    shm_wrapper shm { "ps3eye-driver-shm", nullptr, sizeof(ps3eye::shm) };
    settings s;
    frame fr;
    int framerate = 30;
    bool open = false;
    unsigned timecode = 0;

    ps3eye_camera();
    ~ps3eye_camera() override;

    bool start(info& args) override;
    void stop() override;
    bool is_open() override { return open; }

    std::tuple<const frame&, bool> get_frame() override;
    [[nodiscard]] bool show_dialog() override;
};

struct ps3eye_camera_ final : video::impl::camera_
{
    std::vector<QString> camera_names() const override;
    std::unique_ptr<camera> make_camera(const QString& name) override;
    bool show_dialog(const QString& camera_name) override;
    bool can_show_dialog(const QString& camera_name) override;
};
