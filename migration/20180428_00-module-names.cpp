#include "migration.hpp"
#include "options/options.hpp"

using namespace migrations;
using namespace options;

#include "api/plugin-support.hpp"
#include "compat/library-path.hpp"

struct module_names : migration
{
    using dylib_ptr = Modules::dylib_ptr;
    using dylib_list = Modules::dylib_list;

    struct module_type {
        QString name, def;
        dylib_list const& list;
    };

    Modules m { OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH, dylib_load_quiet };

    module_type types[3] {
        { "tracker-dll", "pt", m.trackers() },
        { "protocol-dll", "freetrack", m.protocols() },
        { "filter-dll", "accela", m.filters() },
    };

    bundle b { make_bundle("modules") };

    QString unique_date() const override
    {
        return "20180428_00";
    }

    QString name() const override
    {
        return "module names";
    }

    bool should_run() const override
    {
        for (const module_type& type : types)
        {
            if (!b->contains(type.name))
                continue;

            const QString value = b->get_variant(type.name).value<QString>();

            for (const dylib_ptr& lib : type.list)
            {
                if (value == lib->name && value != lib->module_name)
                    return true;
            }
        }

        return false;
    }

    void run() override
    {
        for (module_type& type : types)
        {
            if (!b->contains(type.name))
                continue;

            const QString value = b->get_variant(type.name).value<QString>();

            bool found = false;

            for (const dylib_ptr& lib : type.list)
            {
                if (value == lib->name && value != lib->module_name)
                {
                    qDebug() << type.name << value << "=>" << lib->module_name;
                    b->store_kv(type.name, lib->module_name);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                qDebug() << type.name << value << "not found";
                b->store_kv(type.name, QVariant());
            }
        }

        b->save();
    }
};

OPENTRACK_MIGRATION(module_names)
