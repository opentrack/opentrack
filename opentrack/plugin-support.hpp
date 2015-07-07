/* Copyright (c) 2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "plugin-api.hpp"
#include "options.hpp"

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QLibrary>
#include <QFrame>
#include <QList>

#include <cstdio>
#include <cinttypes>
#include <iostream>

#include <QCoreApplication>
#include <QFile>
#include <QDir>
#include <QList>
#include <QStringList>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

#if defined(__APPLE__)
#   define OPENTRACK_SONAME "dylib"
#elif defined(_WIN32)
#   define OPENTRACK_SONAME "dll"
#else
#   define OPENTRACK_SONAME "so"
#endif

#include <iostream>

#ifdef _MSC_VER
#   define OPENTRACK_LIB_PREFIX ""
#else
#   define OPENTRACK_LIB_PREFIX "lib"
#endif


extern "C" typedef void* (*OPENTRACK_CTOR_FUNPTR)(void);
extern "C" typedef Metadata* (*OPENTRACK_METADATA_FUNPTR)(void);

struct dylib {
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
    
#if defined(_WIN32)
        QString fullPath = QCoreApplication::applicationDirPath() + "/" + this->filename;
        handle = new QLibrary(fullPath);
    
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
#else
        QByteArray latin1 = QFile::encodeName(filename);
        handle = dlopen(latin1.constData(),
#   if defined(__APPLE__)
                    RTLD_LOCAL|RTLD_FIRST|RTLD_NOW
#   else
                    RTLD_NOW|RTLD_GLOBAL|RTLD_NODELETE
#   endif
                        );
    
        struct _foo {
            static bool err(void*& handle)
            {
                const char* err = dlerror();
                if (err)
                {
                    fprintf(stderr, "Error, ignoring: %s\n", err);
                    fflush(stderr);
                    if (handle)
                        dlclose(handle);
                    handle = nullptr;
                    return true;
                }
                return false;
            }
        };
    
        if (handle)
        {
            if (_foo::err(handle))
                return;
            Dialog = (OPENTRACK_CTOR_FUNPTR) dlsym(handle, "GetDialog");
            if (_foo::err(handle))
                return;
            Constructor = (OPENTRACK_CTOR_FUNPTR) dlsym(handle, "GetConstructor");
            if (_foo::err(handle))
                return;
            Meta = (OPENTRACK_METADATA_FUNPTR) dlsym(handle, "GetMetadata");
            if (_foo::err(handle))
                return;
        } else {
            (void) _foo::err(handle);
            return;
        }
#endif
    
        auto m = mem<Metadata>(Meta());
    
        icon = m->icon();
        name = m->name();
    }
    ~dylib()
    {
#if defined(_WIN32)
        if (handle)
            delete handle;
#else
        if (handle)
            (void) dlclose(handle);
#endif
    }
    
    static QList<mem<dylib>> enum_libraries()
    {
        const char* filters_n[] = { "opentrack-filter-*.",
                                    "opentrack-tracker-*.",
                                    "opentrack-proto-*."
                                  };
        const Type filters_t[] = { Filter, Tracker, Protocol };
    
        QDir settingsDir( QCoreApplication::applicationDirPath() );
    
        QList<mem<dylib>> ret;
    
        for (int i = 0; i < 3; i++)
        {
            QString filter = filters_n[i];
            auto t = filters_t[i];
            QStringList filenames = settingsDir.entryList(QStringList { OPENTRACK_LIB_PREFIX + filter + OPENTRACK_SONAME },
                                                          QDir::Files,
                                                          QDir::Name);
            for (int i = 0; i < filenames.size(); i++) {
                QIcon icon;
                QString longName;
                QString str = filenames.at(i);
                auto lib = std::make_shared<dylib>(str, t);
                qDebug() << "Loading" << str;
                std::cout.flush();
                if (!get_metadata(lib, longName, icon))
                    continue;
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
#if defined(_WIN32)
    QLibrary* handle;
#else
    void* handle;
#endif
    
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
    
    QList<mem<dylib>> filter(dylib::Type t)
    {
        QList<mem<dylib>> ret;
        for (auto x : module_list)
            if (x->type == t)
                ret.push_back(x);
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
