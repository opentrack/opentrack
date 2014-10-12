#pragma once

#include "facetracknoir/plugin-api.hpp"

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

extern "C" typedef void* (*CTOR_FUNPTR)(void);
extern "C" typedef Metadata* (*METADATA_FUNPTR)(void);
extern "C" typedef void* (*DIALOG_FUNPTR)(void);

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


// XXX TODO it can die if running tracker state separated into class -sh 20141004
class IDynamicLibraryProvider {
public:
    virtual DynamicLibrary* current_tracker1() = 0;
    virtual DynamicLibrary* current_protocol() = 0;
    virtual DynamicLibrary* current_filter() = 0;
    virtual QFrame* get_video_widget() = 0;
};
