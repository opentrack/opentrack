#include "camera.hpp"

#include <algorithm>
#include <utility>
#include <QMutex>

std::pair<std::vector<std::unique_ptr<video::impl::camera_>>&, QMutex&> get_metadata()
{
    static std::vector<std::unique_ptr<video::impl::camera_>> metadata;
    static QMutex mtx;
    return { metadata, mtx };
}

namespace video::impl {

camera_::camera_() = default;
camera_::~camera_() = default;

camera::camera() = default;
camera::~camera() = default;

void register_camera(std::unique_ptr<impl::camera_> camera)
{
    auto [metadata, mtx] = get_metadata();
    QMutexLocker l(&mtx);
    metadata.push_back(std::move(camera));
}

} // ns video::impl

namespace video {

bool show_dialog(const QString& camera_name)
{
    auto [metadata, mtx] = get_metadata();
    QMutexLocker l(&mtx);

    for (auto& camera : metadata)
        for (const QString& name : camera->camera_names())
            if (name == camera_name)
                return camera->show_dialog(camera_name);

    return false;
}

std::unique_ptr<camera_impl> make_camera_(const QString& name)
{
    auto [metadata, mtx] = get_metadata();
    QMutexLocker l(&mtx);

    for (auto& camera : metadata)
        for (const QString& name_ : camera->camera_names())
            if (name_ == name)
                return camera->make_camera(name_);

    return nullptr;
}

std::unique_ptr<camera_impl> make_camera(const QString& name)
{
    if (auto ret = make_camera_(name))
        return ret;

    auto [metadata, mtx] = get_metadata();
    for (auto& camera : metadata)
        for (const QString& name_ : camera->camera_names())
            if (auto ret = camera->make_camera(name_))
                return ret;

    return nullptr;
}

std::vector<QString> camera_names()
{
    auto [metadata, mtx] = get_metadata();

    QMutexLocker l(&mtx);
    std::vector<QString> names; names.reserve(32);

    for (auto& camera : metadata)
        for (const QString& name : camera->camera_names())
            if (std::find(names.cbegin(), names.cend(), name) == names.cend())
                names.push_back(name);

    std::sort(names.begin(), names.end());
    return names;
}

} // ns video
