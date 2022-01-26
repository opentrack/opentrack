#ifdef _WIN32
#include "migration.hpp"
#include "options/options.hpp"
#include "compat/camera-names.hpp"

using namespace migrations;
using namespace options;

#include "api/plugin-support.hpp"
#include "compat/library-path.hpp"
#include <tuple>
#include <QString>

static const std::tuple<const char*, const char*> modules[] = {
    { "tracker-aruco",      "camera-name" },
    { "tracker-easy",       "camera-name" },
    { "neuralnet-tracker",  "camera-name" },
    { "tracker-pt",         "camera-name" },
};

struct win32_camera_name : migration
{
    QString unique_date() const override
    {
        return "20220126_00";
    }

    QString name() const override
    {
        return "camera name";
    }

    bool should_run() const override
    {
        for (const auto& [name, prop] : modules)
        {
            bundle b { make_bundle(name) };
            QString str = b->get_variant(prop).toString();
            if (!str.isEmpty() && !str.contains(" ["))
                return true;
        }
        return false;
    }

    void run() override
    {
        auto cameras = get_camera_names();

        for (const auto& [bundle_name, prop] : modules)
        {
            bundle b { make_bundle(bundle_name) };
            QString name = b->get_variant(prop).toString();
            if (name.isEmpty() || name.contains(" ["))
                continue;
            auto full_name_of = [&](const QString& str) {
                QString ret = str;
                auto prefix = str + " [";
                for (const auto& [x, _] : cameras)
                {
                    if (x == str)
                        return str;
                    if (x.startsWith(prefix))
                        ret = x;
                }
                return ret;
            };
            auto full_name = full_name_of(name);
            if (name != full_name)
            {
                b->store_kv(prop, full_name);
                b->save();
            }
        }
    }
};

OPENTRACK_MIGRATION(win32_camera_name)

#endif
