#pragma once

#include "video/camera.hpp"

#if 0

namespace video {

struct OTR_VIDEO_EXPORT camera_
{
    camera_();
    virtual ~camera_();

    virtual std::vector<QString> camera_names() const = 0;
    virtual std::unique_ptr<camera> make_camera(const QString& name) = 0;
    virtual bool show_dialog(const QString& camera_name) = 0;
    virtual bool can_show_dialog(const QString& camera_name) = 0;
};

struct OTR_VIDEO_EXPORT camera
{
    struct info final
    {
        // TODO: expose FOV-based focal length for regular webcams
        int width = 0, height = 0, fps = 0;
        double fx = 0, fy = 0;          // focal length
        double P_x = 0, P_y = 0;        // principal point
        double dist_c[8] {};            // distortion coefficients
    };

    camera();
    virtual ~camera();

    [[nodiscard]] virtual bool start(info& args) = 0;
    virtual void stop() = 0;
    virtual bool is_open() = 0;

    virtual std::tuple<const frame&, bool> get_frame() = 0;
    [[nodiscard]] virtual bool show_dialog() = 0;
};

} // ns video

#endif

namespace video::impl {
struct ps3eye_camera_ : camera_
{
    std::vector<QString> camera_names() const override;
    std::unique_ptr<camera> make_camera(const QString& name) override;
    bool show_dialog(const QString& camera_name) override;
    bool can_show_dialog(const QString& camera_name) override;
};
} // ns video::impl
