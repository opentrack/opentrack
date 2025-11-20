/* Copyright (c) 2025
 *
 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#pragma once
#include "api/plugin-api.hpp"
#include "options/options.hpp"
#include "ui_smoothtrack-controls.h"
#include <QTcpSocket>
#include <QThread>
#include <cmath>

extern "C"
{
#include <usbmuxd.h>
}

using namespace options;

struct settings : opts
{
    settings() : opts("smoothtrack-tracker") {}
};

class smoothtrack : protected QThread, public ITracker
{
    Q_OBJECT
public:
    smoothtrack();
    ~smoothtrack() override;
    module_status start_tracker(QFrame*) override;
    void data(double* data) override;

protected:
    void run() override;

private:
    // The port the app listens on once "Activate USB Connection" is tapped.
    // It isn't documented anywhere, isn't configurable in the app, and is not
    // user-configurable here either -- a wrong value here just means a
    // connection refused with no way to tell why. SmoothTrack's own USB
    // tunnel tool pins it the same way, by shelling out to
    // `iproxy.exe 47047 47047`, which is the same usbmuxd forward this
    // module makes directly.
    static constexpr quint16 device_port = 47047;

    QTcpSocket sock;
    double last_recv_pose[6];
    QMutex mutex;
    settings s;

    bool connect_to_device();
};

class dialog_smoothtrack : public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_smoothtrack();
    void register_tracker(ITracker*) override {}
    void unregister_tracker() override {}

private:
    Ui::UISmoothTrackControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class smoothtrack_metadata : public Metadata
{
    Q_OBJECT

    QString name() { return tr("SmoothTrack (USB)"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};
