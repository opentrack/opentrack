#pragma once

#include "video/camera.hpp"
#include "shm-layout.hpp"
#include "compat/shm.h"
#include "options/options.hpp"
#include "compat/macros.h"
#include "compat/timer.hpp"
#include "ui_dialog.h"

#include <optional>
#include <QProcess>
#include <QTimer>

using namespace options;

using video::impl::camera;
using video::impl::camera_;
using video::frame;

struct settings final
{
    bundle b = make_bundle("video-ps3eye");
    shm_wrapper shm { "ps3eye-driver-shm", nullptr, sizeof(ps3eye::shm) };

    value<slider_value> exposure{b, "exposure", {255, 0, 255}};
    value<slider_value> gain{b, "gain", {30, 0, 63}};

    void set_exposure();
    void set_gain();
};

class dialog final : public QWidget
{
    Q_OBJECT
    Ui_Dialog ui;
    settings s;
    QTimer t{this};

    shm_wrapper shm { "ps3eye-driver-shm", nullptr, sizeof(ps3eye::shm) };

    void do_ok() { s.b->save(); close(); deleteLater(); }
    void do_cancel() { s.b->reload(); close(); deleteLater(); }

protected:
    void closeEvent(QCloseEvent*) override { do_cancel(); if (t.isActive()) { s.set_exposure(); s.set_gain(); } }

public:
    explicit dialog(QWidget* parent = nullptr);
    static void show_open_failure_msgbox(const QString& error);
};

struct ps3eye_camera final : video::impl::camera
{
    std::optional<QProcess> wrapper;
    shm_wrapper shm { "ps3eye-driver-shm", nullptr, sizeof(ps3eye::shm) };
    settings s;
    frame fr;
    Timer t;
    unsigned char data[640 * 480 * ps3eye::num_channels] = {};
    int framerate = 30, sleep_ms = 1;
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
