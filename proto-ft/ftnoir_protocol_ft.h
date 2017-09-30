/* Copyright (c) 2013-2015 Stanislaw Halik <sthalik@misaki.pl>
 * Copyright (c) 2015 Wim Vriend
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once
#include "ui_ftnoir_ftcontrols.h"
#include "api/plugin-api.hpp"
#include <QMessageBox>
#include <QSettings>
#include <QLibrary>
#include <QProcess>
#include <QDebug>
#include <QFile>
#include <QString>
#include <QMutex>
#include <QMutexLocker>

#include <cinttypes>
#include "freetrackclient/fttypes.h"

#include "compat/shm.h"
#include "options/options.hpp"
#include "mutex.hpp"

#include <memory>

using namespace options;

struct settings : opts {
    value<int> intUsedInterface;
    value<bool> useTIRViews, close_protocols_on_exit;
    settings() :
        opts("proto-freetrack"),
        intUsedInterface(b, "used-interfaces", 0),
        useTIRViews(b, "use-memory-hacks", false),
        close_protocols_on_exit(b, "close-protocols-on-exit", false)
    {}
};

typedef void (__stdcall *importTIRViewsStart)(void);
typedef void (__stdcall *importTIRViewsStop)(void);

class freetrack : public IProtocol
{
public:
    freetrack();
    ~freetrack() override;
    bool correct();
    void pose( const double *headpose );
    QString game_name() override {
        QMutexLocker foo(&game_name_mutex);
        return connected_game;
    }
private:
    settings s;
    shm_wrapper shm;
    FTHeap volatile *pMemData;

    QLibrary FTIRViewsLib;
    QProcess dummyTrackIR;
    importTIRViewsStart viewsStart;
    importTIRViewsStop viewsStop;

    int intGameID;
    QString connected_game;
    QMutex game_name_mutex;
    static check_for_first_run runonce_check;

    void start_tirviews();
    void start_dummy();
    static float degrees_to_rads(double degrees);

public:
    static void set_protocols(bool ft, bool npclient);
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

class freetrackDll : public Metadata
{
public:
    QString name() { return QString(QCoreApplication::translate("freetrackDll", "freetrack 2.0 Enhanced")); }
    QIcon icon() { return QIcon(":/images/freetrack.png"); }
};
