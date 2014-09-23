#include <cstdio>
#include "plugin-support.h"
#include <QCoreApplication>
#include <QFile>

#ifndef _WIN32
#   include <dlfcn.h>
#endif

SelectedLibraries* Libraries = NULL;

SelectedLibraries::~SelectedLibraries()
{
    if (pTracker) {
        delete pTracker;
        pTracker = NULL;
    }

    if (pFilter)
        delete pFilter;

    if (pProtocol)
        delete pProtocol;
}

SelectedLibraries::SelectedLibraries(IDynamicLibraryProvider* mainApp) :
    pTracker(NULL), pFilter(NULL), pProtocol(NULL)
{
    correct = false;
    if (!mainApp)
        return;
    CTOR_FUNPTR ptr;
    DynamicLibrary* lib;

    lib = mainApp->current_tracker1();

    if (lib && lib->Constructor) {
        ptr = (CTOR_FUNPTR) lib->Constructor;
        pTracker = (ITracker*) ptr();
    }

    lib = mainApp->current_protocol();

    if (lib && lib->Constructor) {
        ptr = (CTOR_FUNPTR) lib->Constructor;
        pProtocol = (IProtocol*) ptr();
    }

    lib = mainApp->current_filter();

    if (lib && lib->Constructor) {
        ptr = (CTOR_FUNPTR) lib->Constructor;
        pFilter = (IFilter*) ptr();
    }

    if (pProtocol)
        if(!pProtocol->checkServerInstallationOK())
            return;
    if (pTracker) {
        pTracker->StartTracker( mainApp->get_video_widget() );
    }

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
    
    Dialog = (DIALOG_FUNPTR) handle->resolve("GetDialog");
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
        Dialog = (DIALOG_FUNPTR) dlsym(handle, "GetDialog");
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
