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
#include <QNetworkAccessManager>
#include <QTimer>
#include "ui_s2bot-controls.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<int> freq, idx_x, idx_y, idx_z;
    value<int> add_yaw, add_pitch, add_roll;
    settings() :
        opts("s2bot-tracker"),
        freq(b, "freq", 30),
        idx_x(b, "axis-index-x", 0),
        idx_y(b, "axis-index-y", 1),
        idx_z(b, "axis-index-z", 2),
        add_yaw(b, "add-yaw-degrees", 0),
        add_pitch(b, "add-pitch-degrees", 0),
        add_roll(b, "add-roll-degrees", 0)
    {}
};

class tracker_s2bot : public ITracker, private virtual QThread
{
public:
    tracker_s2bot();
    ~tracker_s2bot() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
protected:
    void run() override;
private:
    double pose[6];
	QTimer timer;
    settings s;
    QMutex mtx;
	std::unique_ptr<QNetworkAccessManager> m_nam;

};

class dialog_s2bot : public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_s2bot();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
    Ui::UI_s2bot_dialog ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class meta_s2bot : public Metadata
{
public:
    QString name() { return otr_tr("S2Bot receiver"); }
    QIcon icon() { return QIcon(":/s2bot.png"); }
};

