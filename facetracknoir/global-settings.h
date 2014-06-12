#pragma once

#if defined(_WIN32)
#   define CALLING_CONVENTION_SUFFIX_VOID_FUNCTION "@0"
#   ifdef _MSC_VER
#       define MAYBE_STDCALL_UNDERSCORE "_"
#else
#       define MAYBE_STDCALL_UNDERSCORE ""
#   endif
#else
#   define CALLING_CONVENTION_SUFFIX_VOID_FUNCTION ""
#   define MAYBE_STDCALL_UNDERSCORE ""
#endif

#ifdef _MSC_VER
#   define virt_override
#else
#   define virt_override override
#endif

#include <cstdio>

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QLibrary>
#include <QFrame>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "ftnoir_protocol_base/ftnoir_protocol_base.h"

#if defined(_WIN32)
#   define CALLING_CONVENTION __stdcall
#else
#   define CALLING_CONVENTION
#endif

class IDynamicLibraryProvider;

struct SelectedLibraries {
public:
    ITracker* pTracker;
    ITracker* pSecondTracker;
    IFilter* pFilter;
    IProtocol* pProtocol;
    SelectedLibraries(IDynamicLibraryProvider* main = NULL);
    ~SelectedLibraries();
    bool correct;
};

extern SelectedLibraries* Libraries;

struct Metadata;

extern "C" typedef void* (CALLING_CONVENTION * NULLARY_DYNAMIC_FUNCTION)(void);
extern "C" typedef Metadata* (CALLING_CONVENTION* METADATA_FUNCTION)(void);
extern "C" typedef void* (CALLING_CONVENTION* SETTINGS_FUNCTION)(void);

class DynamicLibrary {
public:
    DynamicLibrary(const QString& filename);
    virtual ~DynamicLibrary();
    SETTINGS_FUNCTION Dialog;
    NULLARY_DYNAMIC_FUNCTION Constructor;
    METADATA_FUNCTION Metadata;
    QString filename;
private:
#if defined(_WIN32)
    QLibrary* handle;
#else
    void* handle;
#endif
};

struct Metadata
{
    Metadata() {}
    virtual ~Metadata() {}

    virtual void getFullName(QString *strToBeFilled) = 0;
    virtual void getShortName(QString *strToBeFilled) = 0;
    virtual void getDescription(QString *strToBeFilled) = 0;
    virtual void getIcon(QIcon *icon) = 0;
};

class IDynamicLibraryProvider {
public:
    virtual DynamicLibrary* current_tracker1() = 0;
    virtual DynamicLibrary* current_tracker2() = 0;
    virtual DynamicLibrary* current_protocol() = 0;
    virtual DynamicLibrary* current_filter() = 0;
    virtual QFrame* get_video_widget() = 0;
};
