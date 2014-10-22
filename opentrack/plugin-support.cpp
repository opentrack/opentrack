#include <cstdio>
#include <cinttypes>
#include "plugin-support.h"
#include <QCoreApplication>
#include <QFile>
#include <QDir>

#include <iostream>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

SelectedLibraries::~SelectedLibraries()
{
}

template<typename t>
static ptr<t> make_instance(ptr<dylib> lib)
{
    ptr<t> ret;
    if (lib != nullptr && lib->Constructor)
    {
        qDebug() << "dylib" << (lib ? lib->filename : "<null>") << "ptr" << (intptr_t) lib->Constructor;
        std::cout.flush();
        ret = ptr<t>(reinterpret_cast<t*>(reinterpret_cast<CTOR_FUNPTR>(lib->Constructor)()));
    }
    return ret;
}

SelectedLibraries::SelectedLibraries(QFrame* frame, dylibptr t, dylibptr p, dylibptr f) :
    pTracker(nullptr),
    pFilter(nullptr),
    pProtocol(nullptr),
    correct(false)
{
    pTracker = make_instance<ITracker>(t);
    pProtocol = make_instance<IProtocol>(p);
    pFilter = make_instance<IFilter>(f);

    if (!pTracker || !pProtocol)
    {
        qDebug() << "dylib load failure";
        return;
    }

    if(!pProtocol->correct())
    {
        qDebug() << "protocol load failure";
        return;
    }

    qDebug() << "start tracker with frame" << (intptr_t)frame;
    std::cout.flush();

    pTracker->start_tracker(frame);

    correct = true;
}

#if defined(__APPLE__)
#   define SONAME "dylib"
#elif defined(_WIN32)
#   define SONAME "dll"
#else
#   define SONAME "so"
#endif

#include <iostream>

#ifdef _MSC_VER
#   error "No support for MSVC anymore"
#else
#   define LIB_PREFIX "lib"
#endif

static bool get_metadata(ptr<dylib> lib, QString& name, QIcon& icon)
{
    Metadata* meta;
    if (!lib->Meta || ((meta = lib->Meta()), !meta))
        return false;
    name = meta->name();
    icon = meta->icon();
    delete meta;
    return true;
}

QList<ptr<dylib>> dylib::enum_libraries()
{
#define BASE "opentrack-"
#define SUFF "-*."
    const char* filters_n[] = { BASE "filter" SUFF,
                                BASE "tracker" SUFF,
                                BASE "proto" SUFF };
    const Type filters_t[] = { Filter, Tracker, Protocol };

    QDir settingsDir( QCoreApplication::applicationDirPath() );

    QList<ptr<dylib>> ret;

    for (int i = 0; i < 3; i++)
    {
        QString filter = filters_n[i];
        auto t = filters_t[i];
        QStringList filenames = settingsDir.entryList(QStringList { LIB_PREFIX + filter + SONAME },
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

dylib::dylib(const QString& filename, Type t) :
    type(t),
    Dialog(nullptr),
    Constructor(nullptr),
    Meta(nullptr)
{
    // otherwise dlopen opens the calling executable
    if (filename.size() == 0)
        return;

    this->filename = filename;
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

    Dialog = (CTOR_FUNPTR) handle->resolve("GetDialog");
    if (_foo::die(handle, !Dialog))
        return;

    Constructor = (CTOR_FUNPTR) handle->resolve("GetConstructor");
    if (_foo::die(handle, !Constructor))
        return;

    Meta = (METADATA_FUNPTR) handle->resolve("GetMetadata");
    if (_foo::die(handle, !Meta))
        return;
#else
    QByteArray latin1 = QFile::encodeName(filename);
    handle = dlopen(latin1.constData(), RTLD_NOW |
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
        Dialog = (CTOR_FUNPTR) dlsym(handle, "GetDialog");
        if (_foo::err(handle))
            return;
        Constructor = (CTOR_FUNPTR) dlsym(handle, "GetConstructor");
        if (_foo::err(handle))
            return;
        Meta = (METADATA_FUNPTR) dlsym(handle, "GetMetadata");
        if (_foo::err(handle))
            return;
    } else {
        (void) _foo::err(handle);
    }
#endif

    auto m = ptr<Metadata>(Meta());

    icon = m->icon();
    name = m->name();
}

dylib::~dylib()
{
#if defined(_WIN32)
    handle->unload();
#else
    if (handle)
        (void) dlclose(handle);
#endif
}
