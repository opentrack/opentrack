#pragma once

#include "facetracknoir/plugin-api.hpp"

#if defined(_WIN32)
#   define CALLING_CONVENTION_SUFFIX_VOID_FUNCTION "@0"
#   ifdef _MSC_VER
#       error "No support for MSVC anymore"
#else
#       define MAYBE_STDCALL_UNDERSCORE ""
#   endif
#else
#   define CALLING_CONVENTION_SUFFIX_VOID_FUNCTION ""
#   define MAYBE_STDCALL_UNDERSCORE ""
#endif

#include <cstdio>

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QLibrary>
#include <QFrame>

class IDynamicLibraryProvider;

struct SelectedLibraries {
public:
    ITracker* pTracker;
    IFilter* pFilter;
    IProtocol* pProtocol;
    SelectedLibraries(IDynamicLibraryProvider* main = NULL);
    ~SelectedLibraries();
    bool correct;
};

extern SelectedLibraries* Libraries;

struct Metadata;

extern "C" typedef void* (CALLING_CONVENTION * CTOR_FUNPTR)(void);
extern "C" typedef Metadata* (CALLING_CONVENTION* METADATA_FUNPTR)(void);
extern "C" typedef void* (CALLING_CONVENTION* DIALOG_FUNPTR)(void);

class DynamicLibrary {
public:
    DynamicLibrary(const QString& filename);
    ~DynamicLibrary();
    DIALOG_FUNPTR Dialog;
    CTOR_FUNPTR Constructor;
    METADATA_FUNPTR Metadata;
    QString filename;
private:
#if defined(_WIN32)
    QLibrary* handle;
#else
    void* handle;
#endif
};


// merely to break a circular header dependency -sh
class IDynamicLibraryProvider {
public:
    virtual DynamicLibrary* current_tracker1() = 0;
    virtual DynamicLibrary* current_protocol() = 0;
    virtual DynamicLibrary* current_filter() = 0;
    virtual QFrame* get_video_widget() = 0;
};
