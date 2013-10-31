#pragma once

#include <QFrame>
#include <QDir>
#include <QList>
#include <QStringList>
#include <QDebug>
#include <QIcon>
#include <QShowEvent>
#include <iostream>
#include <cstring>
#include <QString>
#include <QApplication>
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "facetracknoir/global-settings.h"
#include <memory>

typedef ITracker* opentrack_tracker;

class opentrack_meta {
public:
    QString path;
    std::shared_ptr<DynamicLibrary> lib;

    opentrack_meta(QString& path, DynamicLibrary* lib) :
        path(path), lib(lib)
    {}
};

class MyFrame : public QFrame {
    Q_OBJECT
public:
    MyFrame(void* parent)
    {
        if (parent == (void*) -1)
        {
            show();
            setVisible(false);
            hide();
        }
        else
        {
            create((WId) parent);
        }
    }
    explicit MyFrame() {}
};

typedef class opentrack_ctx {
public:
    QApplication app;
    char** list;
    QList<opentrack_meta> meta_list;
    MyFrame fake_frame;
    opentrack_ctx(int argc, char** argv, void* window_parent);
    ~opentrack_ctx();
} *opentrack;
