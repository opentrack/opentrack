/* Copyright (c) 2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "plugin-api.hpp"
#include "opentrack-compat/options.hpp"
#include "library-path.hpp"

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
    enum Type { Filter, Tracker, Protocol };

    dylib(const QString& filename, Type t) :
        type(t),
        filename(filename),
        Dialog(nullptr),
        Constructor(nullptr),
        Meta(nullptr),
        handle(nullptr)
    {
        // otherwise dlopen opens the calling executable
        if (filename.size() == 0)
            return;

        QString fullPath = QCoreApplication::applicationDirPath() + "/" + filename;
        handle = new QLibrary(fullPath);
        handle->setLoadHints(QLibrary::PreventUnloadHint | handle->loadHints());

        struct _foo {
            static bool die(QLibrary*& l, bool failp)
            {
                if (failp)
                {
                    qDebug() << "failed" << l->errorString();
                    delete l;
                    l = nullptr;
                }
                return failp;
            }
        };

        if (_foo::die(handle, !handle->load()))
            return;

        Dialog = (OPENTRACK_CTOR_FUNPTR) handle->resolve("GetDialog");
        if (_foo::die(handle, !Dialog))
            return;

        Constructor = (OPENTRACK_CTOR_FUNPTR) handle->resolve("GetConstructor");
        if (_foo::die(handle, !Constructor))
            return;

        Meta = (OPENTRACK_METADATA_FUNPTR) handle->resolve("GetMetadata");
        if (_foo::die(handle, !Meta))
            return;

        auto m = mem<Metadata>(Meta());

        icon = m->icon();
        name = m->name();
    }
    ~dylib()
    {
        if (handle)
            delete handle;
    }

    static QList<mem<dylib>> enum_libraries()
    {
        const char* filters_n[] = { OPENTRACK_SOLIB_PREFIX "opentrack-filter-*." OPENTRACK_SOLIB_EXT,
                                    OPENTRACK_SOLIB_PREFIX "opentrack-tracker-*." OPENTRACK_SOLIB_EXT,
                                    OPENTRACK_SOLIB_PREFIX "opentrack-proto-*." OPENTRACK_SOLIB_EXT,
                                  };
        const Type filters_t[] = { Filter, Tracker, Protocol };

        static const QString libexec_path(QStringLiteral("./") + opentrack_library_path);

        QDir settingsDir(libexec_path);

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
                auto lib = std::make_shared<dylib>(libexec_path + QStringLiteral("/") + filename, t);
                qDebug() << "Loading" << filename;
                std::cout.flush();
                if (!get_metadata(lib, longName, icon))
                    continue;
                using d = const mem<dylib>&;
                if (std::any_of(ret.cbegin(),
                                ret.cend(),
                                [&](d a) {return a->type == lib->type && a->name == lib->name;}))
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
    QLibrary* handle;

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

struct Modules {
    Modules() :
        module_list(dylib::enum_libraries()),
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
