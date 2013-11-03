#include "global-settings.h"

#if !(defined(_WIN32))
#   include <dlfcn.h>
#endif

SelectedLibraries* Libraries = NULL;

SelectedLibraries::~SelectedLibraries()
{
    if (pTracker) {
        delete pTracker;
        pTracker = NULL;
    }

    if (pSecondTracker) {
        delete pSecondTracker;
        pSecondTracker = NULL;
    }

    if (pFilter)
        delete pFilter;

    if (pProtocol)
        delete pProtocol;
}

SelectedLibraries::SelectedLibraries(IDynamicLibraryProvider* mainApp) :
    pTracker(NULL), pSecondTracker(NULL), pFilter(NULL), pProtocol(NULL)
{
    correct = false;
    if (!mainApp)
        return;
    NULLARY_DYNAMIC_FUNCTION ptr;
    DynamicLibrary* lib;

    lib = mainApp->current_tracker1();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pTracker = (ITracker*) ptr();
    }

    lib = mainApp->current_tracker2();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pSecondTracker = (ITracker*) ptr();
    }

    lib = mainApp->current_protocol();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pProtocol = (IProtocol*) ptr();
    }

    lib = mainApp->current_filter();

    if (lib && lib->Constructor) {
        ptr = (NULLARY_DYNAMIC_FUNCTION) lib->Constructor;
        pFilter = (IFilter*) ptr();
    }

    // Check if the Protocol-server files were installed OK.
    // Some servers also create a memory-mapping, for Inter Process Communication.
    // The handle of the MainWindow is sent to 'The Game', so it can send a message back.

    if (pProtocol)
        if(!pProtocol->checkServerInstallationOK())
            return;

    // retrieve pointers to the User Interface and the main Application
    if (pTracker) {
        pTracker->StartTracker( mainApp->get_video_widget() );
    }
    if (pSecondTracker) {
        pSecondTracker->StartTracker( mainApp->get_video_widget() );
    }

    correct = true;
}

DynamicLibrary::DynamicLibrary(const QString& filename)
{
    this->filename = filename;
#if defined(_WIN32)
    QString fullPath = QCoreApplication::applicationDirPath() + "/" + this->filename;
    handle = new QLibrary(fullPath);
    Dialog = (SETTINGS_FUNCTION) handle->resolve(MAYBE_STDCALL_UNDERSCORE "GetDialog" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
    Constructor = (NULLARY_DYNAMIC_FUNCTION) handle->resolve(MAYBE_STDCALL_UNDERSCORE "GetConstructor" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
    Metadata = (METADATA_FUNCTION) handle->resolve(MAYBE_STDCALL_UNDERSCORE "GetMetadata" CALLING_CONVENTION_SUFFIX_VOID_FUNCTION);
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
    if (handle)
    {
        fprintf(stderr, "Error, if any: %s\n", dlerror());
        fflush(stderr);
        Dialog = (SETTINGS_FUNCTION) dlsym(handle, "GetDialog");
        fprintf(stderr, "Error, if any: %s\n", dlerror());
        fflush(stderr);
        Constructor = (NULLARY_DYNAMIC_FUNCTION) dlsym(handle, "GetConstructor");
        fprintf(stderr, "Error, if any: %s\n", dlerror());
        fflush(stderr);
        Metadata = (METADATA_FUNCTION) dlsym(handle, "GetMetadata");
        fprintf(stderr, "Error, if any: %s\n", dlerror());
        fflush(stderr);
    } else {
        fprintf(stderr, "Error, if any: %s\n", dlerror());
        fflush(stderr);
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
