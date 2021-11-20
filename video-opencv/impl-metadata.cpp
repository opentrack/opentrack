#include "impl.hpp"
#include "compat/camera-names.hpp"
#include "video-property-page.hpp"

namespace opencv_camera_impl {

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
    std::vector<std::tuple<QString, int>> names = get_camera_names();
    std::vector<QString> ret;
    for (const auto& [str, idx] : names)
        ret.push_back(str);
    return ret;
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

OTR_REGISTER_CAMERA(metadata)

} // ns opencv_camera_impl
