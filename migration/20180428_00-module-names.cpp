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
        return true;
    }

    void run() override
    {
        struct module_type {
            QString name, def;
            dylib_list const& list;
        };

        Modules m { OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH };

        module_type types[3] {
            { "tracker-dll", "pt", m.trackers() },
            { "protocol-dll", "freetrack", m.protocols() },
            { "filter-dll", "accela", m.filters() },
        };

        bundle b = make_bundle("modules");

        for (module_type& type : types)
        {
            QByteArray n = type.name.toUtf8();

            if (!b->contains(type.name))
            {
                qDebug() << n.constData() << "=>" << "EMPTY";
                b->store_kv(type.name, type.def);
                continue;
            }

            QString value = b->get_variant(type.name).value<QString>();

            bool found = false;

            for (dylib_ptr lib : type.list)
            {
                if (value == lib->name)
                {
                    qDebug() << n.constData() << "=>" << lib->module_name;
                    b->store_kv(type.name, lib->module_name);
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                qDebug() << n.constData() << "=>" << "not found" << value;
                b->store_kv(type.name, type.def);
            }
        }

        b->save();
    }
};

OPENTRACK_MIGRATION(module_names);
