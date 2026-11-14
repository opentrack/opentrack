#pragma once
#include "ui_livelink.h"
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
        opts("livelink-tracker"),
        port(b, "port", 11111),
        add_yaw(b, "add-yaw", 0),
        add_pitch(b, "add-pitch", 0),
        add_roll(b, "add-roll", 0)
    {}
};

class livelink : protected QThread, public ITracker
{
    Q_OBJECT
public:
    livelink();
    ~livelink() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
protected:
    void run() override;
private:
    QUdpSocket sock;
    float last_recv_pose[3], last_recv_pose2[3];
    QMutex mutex;
    settings s;
};

class dialog_livelink: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_livelink();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
    Ui::UILiveLink ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class meta_livelink : public Metadata
{
    Q_OBJECT

    QString name() { return tr("IPhone LiveLink Tracker"); }
    QIcon icon() { return QIcon(":/livelink.png"); }
};

