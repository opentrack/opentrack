#pragma once

#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "facetracknoir/global-settings.h"
#include <QString>
#include <QDir>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QIcon>
#include <iostream>
#include <cstring>

#ifdef __GNUC__
#   define OPENTRACK_HIDDEN __attribute__((visibility ("hidden")))
#else
#   define OPENTRACK_HIDDEN
#endif

typedef ITracker* opentrack_tracker;

class OPENTRACK_HIDDEN opentrack_meta {
public:
    Metadata* meta;
    QString path;
    DynamicLibrary* lib;

    opentrack_meta(Metadata* meta, QString& path, DynamicLibrary* lib) :
        meta(meta), path(path), lib(lib)
    {}
    ~opentrack_meta()
    {
        delete meta;
        delete lib;
    }
};

typedef class OPENTRACK_HIDDEN opentrack_ctx {
public:
    QDir dir;
    char** list;
    QList<opentrack_meta> meta_list;
    QFrame fake_frame;
    opentrack_ctx(QDir& dir);
    ~opentrack_ctx();
} *opentrack;
