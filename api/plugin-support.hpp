/* Copyright (c) 2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "plugin-api.hpp"
#include "compat/library-path.hpp"

#include <memory>
#include <algorithm>
#include <cstring>
#include <vector>

#include <QDebug>
#include <QString>
#include <QStringView>
#include <QLatin1StringView>
#include <QLibrary>
#include <QDir>
#include <QIcon>

extern "C" {
    using module_ctor_t = void* (*)(void);
    using module_metadata_t = Metadata_* (*)(void);
}

enum dylib_load_mode : unsigned
{
    dylib_load_norm  = 0,
    dylib_load_quiet = 1 << 0,
    dylib_load_none  = 1 << 1,
};

enum class dylib_type : unsigned
{
    Filter    = 0xdeadbabe,
    Tracker   = 0xcafebeef,
    Protocol  = 0xdeadf00d,
    Video     = 0xbadf00d,
    Invalid   = (unsigned)-1,
};

struct dylib final
{
    dylib(const QString& filename_, dylib_type t, dylib_load_mode load_mode = dylib_load_norm) :
        full_filename(filename_),
        module_name(trim_filename(filename_))
    {
        // otherwise dlopen opens the calling executable
        if (filename_.isEmpty() || module_name.isEmpty())
            return;

        handle.setFileName(filename_);
        handle.setLoadHints(QLibrary::DeepBindHint | QLibrary::ResolveAllSymbolsHint);

#ifdef __clang__
#   pragma clang diagnostic push
#   pragma clang diagnostic ignored "-Wcomma"
#endif

        if (!handle.load())
            goto fail;

        if (!(load_mode & dylib_load_none))
        {
            std::unique_ptr<Metadata_> m;

            if (Dialog = (module_ctor_t) handle.resolve("GetDialog"), !Dialog)
                goto fail;

            if (Constructor = (module_ctor_t) handle.resolve("GetConstructor"), !Constructor)
                goto fail;

            if (Meta = (module_metadata_t) handle.resolve("GetMetadata"), !Meta)
                goto fail;

            m = std::unique_ptr<Metadata_>(Meta());

            if (!m)
            {
                if (!(load_mode & dylib_load_quiet))
                {
                    qDebug() << "library" << module_name << "failed: no metadata";
                    load_mode = dylib_load_quiet;
                }
                goto fail;
            }

            icon = m->icon();
            name = m->name();
        }
        else
            name = module_name;

        type = t;

        return;
#ifdef __clang__
#   pragma clang diagnostic pop
#endif

fail:
        if (!(load_mode & dylib_load_quiet))
            qDebug() << "library" << module_name << "failed:" << handle.errorString();

        Constructor = nullptr;
        Dialog = nullptr;
        Meta = nullptr;

        type = dylib_type::Invalid;
    }

    // QLibrary refcounts the .dll's so don't forcefully unload
    ~dylib() = default;

    dylib_type type = dylib_type::Invalid;
    QString full_filename;
    QString module_name;

    QIcon icon;
    QString name;

    module_ctor_t Dialog = nullptr;
    module_ctor_t Constructor = nullptr;
    module_metadata_t Meta = nullptr;

private:
    QLibrary handle;

    static QString trim_filename(const QString& in_)
    {
        auto in = QStringView{in_};

        const int idx = in.lastIndexOf(QLatin1StringView{"/"});

        if (idx != -1)
        {
            in = in.mid(idx + 1);

            if (in.startsWith(QLatin1StringView{OPENTRACK_LIBRARY_PREFIX}) &&
                in.endsWith(QLatin1StringView{"." OPENTRACK_LIBRARY_EXTENSION}))
            {
                constexpr unsigned pfx_len = sizeof(OPENTRACK_LIBRARY_PREFIX) - 1;
                constexpr unsigned rst_len = sizeof("." OPENTRACK_LIBRARY_EXTENSION) - 1;

                in = in.mid(pfx_len);
                in = in.left(in.size() - rst_len);

                const char* const names[] =
                {
                    OPENTRACK_LIBRARY_PREFIX "opentrack-tracker-",
                    OPENTRACK_LIBRARY_PREFIX "opentrack-proto-",
                    OPENTRACK_LIBRARY_PREFIX "opentrack-filter-",
                    OPENTRACK_LIBRARY_PREFIX "opentrack-video-",
                };

                for (auto name : names)
                {
                    if (in.startsWith(QLatin1StringView{name}))
                        return in.mid((int)std::strlen(name)).toString();
                }
            }
        }
        return {""};
    }
};

struct Modules final
{
    using dylib_ptr = std::shared_ptr<dylib>;
    using dylib_list = std::vector<dylib_ptr>;
    using type = dylib_type;

    Modules(const QString& library_path, dylib_load_mode load_mode = dylib_load_norm) :
        module_list(enum_libraries(library_path, load_mode)),
        filter_modules(filter(type::Filter)),
        tracker_modules(filter(type::Tracker)),
        protocol_modules(filter(type::Protocol)),
        video_modules(filter(type::Video))
    {}
    dylib_list& filters() { return filter_modules; }
    dylib_list& trackers() { return tracker_modules; }
    dylib_list& protocols() { return protocol_modules; }

private:
    dylib_list module_list;
    dylib_list filter_modules;
    dylib_list tracker_modules;
    dylib_list protocol_modules;
    dylib_list video_modules;

    static dylib_list& sorted(dylib_list& xs)
    {
        std::sort(xs.begin(), xs.end(),
                  [&](const dylib_ptr& a, const dylib_ptr& b) {
                      return a->name.toLower() < b->name.toLower();
        });
        return xs;
    }

    dylib_list filter(dylib_type t)
    {
        dylib_list ret; ret.reserve(module_list.size());
        for (const auto& x : module_list)
            if (x->type == t)
                ret.push_back(x);

        return sorted(ret);
    }

    static dylib_list enum_libraries(const QString& library_path,
                                     dylib_load_mode load_mode = dylib_load_norm)
    {
        QDir dir(library_path);
        dylib_list ret;

        const struct filter_ {
            type t = type::Invalid;
            QString glob;
            dylib_load_mode load_mode = dylib_load_norm;
        } filters[] = {
            { type::Filter, OPENTRACK_LIBRARY_PREFIX "opentrack-filter-*." OPENTRACK_LIBRARY_EXTENSION, },
            { type::Tracker, OPENTRACK_LIBRARY_PREFIX "opentrack-tracker-*." OPENTRACK_LIBRARY_EXTENSION, },
            { type::Protocol, OPENTRACK_LIBRARY_PREFIX "opentrack-proto-*." OPENTRACK_LIBRARY_EXTENSION, },
            { type::Video, OPENTRACK_LIBRARY_PREFIX "opentrack-video-*." OPENTRACK_LIBRARY_EXTENSION, dylib_load_none, },
        };

        for (const filter_& filter : filters)
        {
            for (const QString& filename : dir.entryList({ filter.glob }, QDir::Files, QDir::Name))
            {
                dylib_load_mode load_mode_{filter.load_mode | load_mode};
                auto lib = std::make_shared<dylib>(QString("%1/%2").arg(library_path, filename), filter.t, load_mode_);

                if (lib->type == type::Invalid)
                    continue;

                if (std::any_of(ret.cbegin(),
                                ret.cend(),
                                [&lib](const std::shared_ptr<dylib>& a) {
                                    return a->type == lib->type && a->name == lib->name;
                                }))
                {
                    if (!(load_mode & dylib_load_quiet))
                        qDebug() << "duplicate lib" << filename << "ident" << lib->name;
                    continue;
                }

                ret.push_back(std::move(lib));
            }
        }

        return ret;
    }
};

template<typename t>
std::shared_ptr<t> make_dylib_instance(const std::shared_ptr<dylib>& lib)
{
    if (lib != nullptr && lib->Constructor)
        return std::shared_ptr<t>{(t*)lib->Constructor()};
    else
        return nullptr;
}
