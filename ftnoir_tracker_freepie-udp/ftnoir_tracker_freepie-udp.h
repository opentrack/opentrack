/* Copyright (c) 2014 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include <cinttypes>
#include <QUdpSocket>
#include <QThread>
#include "ui_freepie-udp-controls.h"
#include "opentrack/plugin-api.hpp"
#include "opentrack/options.hpp"
using namespace options;

struct settings {
    pbundle b;
    value<int> port, idx_x, idx_y, idx_z;
    settings() :
        b(bundle("freepie-udp-tracker")),
        port(b, "port", 4237),
        idx_x(b, "axis-index-x", 0),
        idx_y(b, "axis-index-y", 1),
        idx_z(b, "axis-index-z", 2)
    {}
};

class TrackerImpl : public ITracker, private QThread
{
public:
    TrackerImpl();
    ~TrackerImpl() override;
    void start_tracker(QFrame *);
    void data(double *data);
protected:
    void run() override;
private:
    double pose[6];
    QUdpSocket sock;
    settings s;
    QMutex mtx;
    volatile bool should_quit;
};

class TrackerDialog : public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerDialog();
    void register_tracker(ITracker *) {}
    void unregister_tracker() {}
private:
    Ui::UI_freepie_udp_dialog ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class TrackerMeta : public Metadata
{
public:
    QString name() { return QString("FreePIE UDP receiver"); }
    QIcon icon() { return QIcon(":/glovepie.png"); }
};

