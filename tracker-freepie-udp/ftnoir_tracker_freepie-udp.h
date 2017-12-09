/* Copyright (c) 2014 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include <cinttypes>
#include <QUdpSocket>
#include <QThread>
#include "ui_freepie-udp-controls.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<int> port, idx_x, idx_y, idx_z;
    value<int> add_yaw, add_pitch, add_roll;
    settings() :
        opts("freepie-udp-tracker"),
        port(b, "port", 5555),
        idx_x(b, "axis-index-x", 0),
        idx_y(b, "axis-index-y", 1),
        idx_z(b, "axis-index-z", 2),
        add_yaw(b, "add-yaw-degrees", 0),
        add_pitch(b, "add-pitch-degrees", 0),
        add_roll(b, "add-roll-degrees", 0)
    {}
};

class tracker_freepie : public ITracker, private QThread
{
public:
    tracker_freepie();
    ~tracker_freepie() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
protected:
    void run() override;
private:
    double pose[6];
    QUdpSocket sock;
    settings s;
    QMutex mtx;
};

class dialog_freepie : public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_freepie();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
    Ui::UI_freepie_udp_dialog ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class meta_freepie : public Metadata
{
public:
    QString name() { return otr_tr("FreePIE UDP receiver"); }
    QIcon icon() { return QIcon(":/glovepie.png"); }
};

