/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2015 Wim Vriend
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once
#include "ui_ftnoir_ftcontrols.h"
#include "opentrack/plugin-api.hpp"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QMutex>
#include <QMutexLocker>
#include "opentrack-compat/shm.h"
#include "opentrack/options.hpp"
#include "freetrackclient/fttypes.h"
using namespace options;

struct settings : opts {
    value<int> intUsedInterface;
    value<bool> useTIRViews;
    settings() :
        opts("proto-freetrack"),
        intUsedInterface(b, "used-interfaces", 0),
        useTIRViews(b, "use-memory-hacks", false)
    {}
};

typedef void (__stdcall *importTIRViewsStart)(void);
typedef void (__stdcall *importTIRViewsStop)(void);

class FTNoIR_Protocol : public IProtocol
{
public:
    FTNoIR_Protocol();
    ~FTNoIR_Protocol() override;
    bool correct();
    void pose( const double *headpose );
    QString game_name() override {
        QMutexLocker foo(&game_name_mutex);
        return connected_game;
    }
private:
    settings s;
    PortableLockedShm shm;
    FTHeap *pMemData;

    QLibrary FTIRViewsLib;
    QProcess dummyTrackIR;
    importTIRViewsStart viewsStart;
    importTIRViewsStop viewsStop;

    int intGameID;
    QString connected_game;
    QMutex game_name_mutex;

    static inline double getRadsFromDegrees(double degrees) { return degrees * 0.017453; }
    void start_tirviews();
    void start_dummy();
};

class FTControls: public IProtocolDialog
{
    Q_OBJECT
public:
    FTControls();
    void register_protocol(IProtocol *) {}
    void unregister_protocol() {}
private:
    Ui::UICFTControls ui;
    settings s;
private slots:
    void selectDLL();
    void doOK();
    void doCancel();
};

class FTNoIR_ProtocolDll : public Metadata
{
public:
    QString name() { return QString("freetrack 2.0 Enhanced"); }
    QIcon icon() { return QIcon(":/images/freetrack.png"); }
};
