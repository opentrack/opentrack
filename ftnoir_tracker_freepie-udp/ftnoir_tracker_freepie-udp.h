/* Copyright (c) 2014 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ui_freepie-udp-controls.h"
#include <QUdpSocket>
#include <QThread>
#include "facetracknoir/plugin-support.h"
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> port;
    value<bool> enable_roll, enable_pitch, enable_yaw;
    settings() :
        b(bundle("freepie-udp-tracker")),
        port(b, "port", 4237),
        enable_roll(b, "enable-roll", true),
        enable_pitch(b, "enable-pitch", true),
        enable_yaw(b, "enable-yaw", true)
    {}
};

class TrackerImpl : public ITracker, protected QThread
{
public:
	TrackerImpl();
    virtual ~TrackerImpl() override;
    void StartTracker(QFrame *);
    void GetHeadPoseData(double *data);
protected:
	void run();
private:
    double pose[6];
    QUdpSocket sock;
    settings s;
    QMutex mtx;
    volatile bool should_quit;
};

class TrackerDialog : public QWidget, public ITrackerDialog
{
    Q_OBJECT
public:
	TrackerDialog();
    void registerTracker(ITracker *) {}
    void unRegisterTracker() {}
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
	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);
};

