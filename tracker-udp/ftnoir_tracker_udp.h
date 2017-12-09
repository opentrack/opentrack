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

class udp : protected QThread, public ITracker
{
public:
    udp();
    ~udp() override;
    module_status start_tracker(QFrame *) override;
    void data(double *data) override;
protected:
    void run() override;
private:
    QUdpSocket sock;
    double last_recv_pose[6], last_recv_pose2[6];
    QMutex mutex;
    settings s;
};

class dialog_udp: public ITrackerDialog
{
    Q_OBJECT
public:
    dialog_udp();
    void register_tracker(ITracker *) override {}
    void unregister_tracker() override {}
private:
    Ui::UICFTNClientControls ui;
    settings s;
private slots:
    void doOK();
    void doCancel();
};

class udpDll : public Metadata
{
public:
    QString name() { return otr_tr("UDP over network"); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};

