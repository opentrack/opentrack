/* Copyright (c) 2014 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include <cinttypes>
#include "ui_edtracker-controls.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
#include <QtGlobal>
#include <QSocketNotifier>
#include <QThread>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

using namespace options;

struct settings : opts {
    value<int> dev, idx_x, idx_y, idx_z;
    value<int> add_yaw, add_pitch, add_roll;
    settings() :
        opts("edtracker-tracker"),
        dev(b, "port", 0),
        idx_x(b, "axis-index-x", 0),
        idx_y(b, "axis-index-y", 1),
        idx_z(b, "axis-index-z", 2),
        add_yaw(b, "add-yaw-degrees", 0),
        add_pitch(b, "add-pitch-degrees", 0),
        add_roll(b, "add-roll-degrees", 0)
    {}
};

struct joys {
    QString name;
    int device;
};
static QList<joys> joys_;
void getJoysticks();

class tracker_edtracker : protected QThread, public ITracker {
    Q_OBJECT
  public:
    tracker_edtracker();
    ~tracker_edtracker() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
    QSocketNotifier *notifier;

  private:
    double pose[6];
    settings s;
    int idevice;

  public slots:
    void readyRead(int /*socket*/);
};

class dialog_edtracker : public ITrackerDialog {
    Q_OBJECT
  public:
    dialog_edtracker();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
    void updateDevices();

  private:
    Ui::UI_edtracker_dialog ui;
    settings s;

  private slots:
    void doOK();
    void doCancel();
};

class meta_edtracker : public Metadata {
    Q_OBJECT

    QString name() {
        return tr("EdTracker receiver");
    }
    QIcon icon() {
        return QIcon(":images/edtracker_logo.png");
    }
};

