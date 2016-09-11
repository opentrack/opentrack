/* Copyright (c) 2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "plugin-api.hpp"
#include "options/options.hpp"

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QLibrary>
#include <QFrame>
#include <QList>

#include <cstdio>
#include <cinttypes>
#include <iostream>
#include <algorithm>

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QList>
#include <QStringList>

#if defined(__APPLE__)
#   define OPENTRACK_SOLIB_EXT "dylib"
#elif defined(_WIN32)
#   define OPENTRACK_SOLIB_EXT "dll"
#else
#   define OPENTRACK_SOLIB_EXT "so"
#endif

#include <iostream>

#ifdef _MSC_VER
#   define OPENTRACK_SOLIB_PREFIX ""
#else
#   define OPENTRACK_SOLIB_PREFIX "lib"
#endif

extern "C" typedef void* (*OPENTRACK_CTOR_FUNPTR)(void);
extern "C" typedef Metadata* (*OPENTRACK_METADATA_FUNPTR)(void);

struct dylib final {
    enum Type : unsigned { Filter, Tracker, Protocol, Invalid = 0xcafebabeU };

    dylib(const QString& filename, Type t) :
        type(Invalid),
        filename(filename),
        Dialog(nullptr),
        Constructor(nullptr),
        Meta(nullptr),
        handle(nullptr)
    {
        // otherwise dlopen opens the calling executable
        if (filename.size() == 0)
            return;

        handle.setFileName(filename);
        handle.setLoadHints(QLibrary::DeepBindHint | QLibrary::LoadHint::ResolveAllSymbolsHint);

        if (check(!handle.load()))
            return;

        if (check((Dialog = (OPENTRACK_CTOR_FUNPTR) handle.resolve("GetDialog"), !Dialog)))
            return;

        if (check((Constructor = (OPENTRACK_CTOR_FUNPTR) handle.resolve("GetConstructor"), !Constructor)))
            return;

        if (check((Meta = (OPENTRACK_METADATA_FUNPTR) handle.resolve("GetMetadata"), !Meta)))
            return;

        auto m = mem<Metadata>(Meta());

        icon = m->icon();
        name = m->name();

        type = t;
    }
    ~dylib()
    {
        if (handle.isLoaded())
        {
            const bool success = handle.unload();
            if (!success)
                qDebug() << "can't unload dylib" << filename << handle.errorString();
        }
    }

    static QList<mem<dylib>> enum_libraries(const QString& library_path)
    {
        const char* filters_n[] = { OPENTRACK_SOLIB_PREFIX "opentrack-filter-*." OPENTRACK_SOLIB_EXT,
                                    OPENTRACK_SOLIB_PREFIX "opentrack-tracker-*." OPENTRACK_SOLIB_EXT,
                                    OPENTRACK_SOLIB_PREFIX "opentrack-proto-*." OPENTRACK_SOLIB_EXT,
                                  };
        const Type filters_t[] = { Filter, Tracker, Protocol };

        QDir settingsDir(library_path);

        QList<mem<dylib>> ret;

        for (int i = 0; i < 3; i++)
        {
            QString glob = filters_n[i];
            Type t = filters_t[i];
            QStringList filenames = settingsDir.entryList(QStringList { glob }, QDir::Files, QDir::Name);

            for (const QString& filename : filenames)
            {
                QIcon icon;
                QString longName;
                auto lib = std::make_shared<dylib>(library_path + filename, t);
                if (!get_metadata(lib, longName, icon))
                    continue;
                if (std::any_of(ret.cbegin(),
                                ret.cend(),
                                [&](mem<dylib> a) {return a->type == lib->type && a->name == lib->name;}))
                {
                    qDebug() << "Duplicate lib" << lib->filename;
                    continue;
                }
                ret.push_back(lib);
            }
        }

        return ret;
    }

    Type type;
    QString filename;

    QIcon icon;
    QString name;

    OPENTRACK_CTOR_FUNPTR Dialog;
    OPENTRACK_CTOR_FUNPTR Constructor;
    OPENTRACK_METADATA_FUNPTR Meta;
private:
    QLibrary handle;

    bool check(bool fail)
    {
        if (fail)
        {
            if (handle.isLoaded())
                (void) handle.unload();

            Constructor = nullptr;
            Dialog = nullptr;
            Meta = nullptr;

            type = Invalid;
        }

        return fail;
    }

    static bool get_metadata(mem<dylib> lib, QString& name, QIcon& icon)
    {
        Metadata* meta;
        if (!lib->Meta || ((meta = lib->Meta()), !meta))
            return false;
        name = meta->name();
        icon = meta->icon();
        delete meta;
        return true;
    }
};

struct Modules final
{
    Modules(const QString& library_path) :
        module_list(dylib::enum_libraries(library_path)),
        filter_modules(filter(dylib::Filter)),
        tracker_modules(filter(dylib::Tracker)),
        protocol_modules(filter(dylib::Protocol))
    {}
    QList<mem<dylib>>& filters() { return filter_modules; }
    QList<mem<dylib>>& trackers() { return tracker_modules; }
    QList<mem<dylib>>& protocols() { return protocol_modules; }
private:
    QList<mem<dylib>> module_list;
    QList<mem<dylib>> filter_modules;
    QList<mem<dylib>> tracker_modules;
    QList<mem<dylib>> protocol_modules;

    template<typename t>
    static void sort(QList<t>& xs)
    {
        std::sort(xs.begin(), xs.end(), [&](const t& a, const t& b) { return a->name.toLower() < b->name.toLower(); });
    }

    QList<mem<dylib>> filter(dylib::Type t)
    {
        QList<mem<dylib>> ret;
        for (auto x : module_list)
            if (x->type == t)
                ret.push_back(x);

        sort(ret);

        return ret;
    }
};

template<typename t>
mem<t> make_dylib_instance(mem<dylib> lib)
{
    mem<t> ret;
    if (lib != nullptr && lib->Constructor)
        ret = mem<t>(reinterpret_cast<t*>(reinterpret_cast<OPENTRACK_CTOR_FUNPTR>(lib->Constructor)()));
    return ret;
}
