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
    value<int> port;
    value<int> add_yaw, add_pitch, add_roll;
    settings()
        : opts("smoothtrack-tracker"), port(b, "port", 47047), add_yaw(b, "add-yaw", 0), add_pitch(b, "add-pitch", 0), add_roll(b, "add-roll", 0)
    {
    }
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
    QTcpSocket* sock = nullptr;
    int usbmuxd_fd = -1;
    double last_recv_pose[6], last_recv_pose2[6];
    QMutex mutex;
    settings s;

    bool connect_to_device();
    void disconnect_from_device();
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

    QString name() { return tr("SmoothTrack iOS"); }
    QIcon icon() { return QIcon(":/images/opentrack.png"); }
};
