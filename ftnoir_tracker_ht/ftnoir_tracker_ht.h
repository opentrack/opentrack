/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "headtracker-ftnoir.h"
#include "ui_ht-trackercontrols.h"
#include "ht_video_widget.h"
#include "compat/compat.h"
#include <QObject>
#include "opentrack/options.hpp"
#include "opentrack/plugin-api.hpp"
#include "opentrack/opencv-camera-dialog.hpp"

#include <QThread>
#include <QMutex>
#include <QMutexLocker>
#include <QHBoxLayout>
#include <QString>

using namespace options;

struct settings : opts {
    value<double> fov;
    value<QString> camera_name;
    value<int> fps, resolution;
    settings() :
        opts("HT-Tracker"),
        fov(b, "fov", 56),
        camera_name(b, "camera-name", ""),
        fps(b, "fps", 0),
        resolution(b, "resolution", 0)
    {}
};

class Tracker : public QThread, public ITracker
{
    Q_OBJECT
public:
    Tracker();
    ~Tracker() override;
    void run() override;
    void start_tracker(QFrame* frame);
    void data(double *data);
    void load_settings(ht_config_t* config);
    headtracker_t* ht;
    QMutex camera_mtx;
private:
    double ypr[6];
    settings s;
    ht_config_t conf;
    HTVideoWidget* videoWidget;
    QHBoxLayout* layout;
    QMutex ypr_mtx, frame_mtx;
    ht_video_t frame;
    volatile bool should_stop;
};

class TrackerControls : public ITrackerDialog, protected camera_dialog<Tracker>
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker * t)
    {
        tracker = static_cast<Tracker*>(t);
    }
    void unregister_tracker() {
        tracker = nullptr;
    }
private:
    Ui::Form ui;
    settings s;
    Tracker* tracker;
private slots:
    void doOK();
    void doCancel();
    void camera_settings();
};

class TrackerDll : public Metadata
{
    QString name() { return QString("ht -- face tracker"); }
    QIcon icon() { return QIcon(":/images/ht.png"); }
};
