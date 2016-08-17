#pragma once
#include "ui_ftnoir_ftnclientcontrols.h"
#include <QUdpSocket>
#include <QThread>
#include <cmath>
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

struct settings : opts {
    value<int> port;
    value<int> add_yaw, add_pitch, add_roll;
    settings() :
        opts("udp-tracker"),
        port(b, "port", 4242),
        add_yaw(b, "add-yaw", 0),
        add_pitch(b, "add-pitch", 0),
        add_roll(b, "add-roll", 0)
    {}
};

class FTNoIR_Tracker : public ITracker, protected QThread
{
public:
    FTNoIR_Tracker();
    ~FTNoIR_Tracker() override;
    void start_tracker(QFrame *) override;
    void data(double *data) override;
protected:
    void run() override;
private:
    QUdpSocket sock;
    double last_recv_pose[6];
    QMutex mutex;
    settings s;
    volatile bool should_quit;
};

class TrackerControls: public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerControls();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
    Ui::UICFTNClientControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class FTNoIR_TrackerDll : public Metadata
{
public:
    QString name() { return QString("UDP sender"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};

