#include <cstdio>
#include <cinttypes>
#include "plugin-support.h"
#include <QCoreApplication>
#include <QFile>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

SelectedLibraries::~SelectedLibraries()
{
}

template<typename t>
static ptr<t> make_instance(ptr<DynamicLibrary> lib)
{
    ptr<t> ret = nullptr;
    if (lib && lib->Constructor)
        ret = ptr<t>(reinterpret_cast<t*>(reinterpret_cast<CTOR_FUNPTR>(lib->Constructor)()));
    qDebug() << "lib" << (lib ? lib->filename : "<null>") << "ptr" << (intptr_t)ret.get();
    return ret;
}

SelectedLibraries::SelectedLibraries(QFrame* frame, dylib t, dylib p, dylib f) :
    pTracker(nullptr),
    pFilter(nullptr),
    pProtocol(nullptr),
    correct(false)
{
    pTracker = make_instance<ITracker>(t);
    pProtocol = make_instance<IProtocol>(p);
    pFilter = make_instance<IFilter>(f);
    
    if (!pTracker|| !pProtocol)
    {
        qDebug() << "load failure tracker" << (intptr_t)pTracker.get() << "protocol" << (intptr_t)pProtocol.get();
        return;
    }

    if (pProtocol)
        if(!pProtocol->correct())
        {
            qDebug() << "protocol load failure";
            return;
        }
    
    pTracker->start_tracker(frame);

    correct = true;
}

DynamicLibrary::DynamicLibrary(const QString& filename) :
    Dialog(nullptr),
    Constructor(nullptr),
    Metadata(nullptr)
{
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
    
    Metadata = (METADATA_FUNPTR) handle->resolve("GetMetadata");
    if (_foo::die(handle, !Metadata))
        return;
#else
    QByteArray latin1 = QFile::encodeName(filename);
    handle = dlopen(latin1.constData(), RTLD_NOW |
#   ifdef __linux
                    RTLD_DEEPBIND
#   elif defined(__APPLE__)
                    RTLD_LOCAL|RTLD_FIRST|RTLD_NOW
#   else
                    0
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
        Metadata = (METADATA_FUNPTR) dlsym(handle, "GetMetadata");
        if (_foo::err(handle))
            return;
    } else {
        (void) _foo::err(handle);
    }
#endif
}

DynamicLibrary::~DynamicLibrary()
{
#if defined(_WIN32)
    handle->unload();
#else
    if (handle)
        (void) dlclose(handle);
#endif
}
