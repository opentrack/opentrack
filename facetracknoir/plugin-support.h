#pragma once

#include "facetracknoir/plugin-api.hpp"

#include <QWidget>
#include <QDebug>
#include <QString>
#include <QLibrary>
#include <QFrame>

#include <memory>
template<typename t> using ptr = std::shared_ptr<t>;

extern "C" typedef void* (*CTOR_FUNPTR)(void);
extern "C" typedef Metadata* (*METADATA_FUNPTR)(void);

class DynamicLibrary {
public:
    DynamicLibrary(const QString& filename);
    ~DynamicLibrary();
    CTOR_FUNPTR Dialog;
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

struct SelectedLibraries {
    using dylib = ptr<DynamicLibrary>;

    ptr<ITracker> pTracker;
    ptr<IFilter> pFilter;
    ptr<IProtocol> pProtocol;
    SelectedLibraries(QFrame* frame, dylib t, dylib p, dylib f);
    SelectedLibraries() : pTracker(nullptr), pFilter(nullptr), pProtocol(nullptr), correct(false) {}
    ~SelectedLibraries();
    bool correct;
};