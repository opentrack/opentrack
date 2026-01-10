#pragma once
#pragma comment(lib, "ws2_32.lib")
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "options/options.hpp"
#include "ui_wxr.h"
using namespace options;

#include <Winsock2.h>
#include <cmath>
#include <condition_variable>
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <io.h>
#include <iostream>
#include <locale>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#include <QThread>
#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

struct settings : opts
{
    value<double> yaw_scale, pitch_scale, roll_scale, yaw_scale_immersive, pitch_scale_immersive, roll_scale_immersive;
    settings()
        : opts("wxr-tracker"),
          yaw_scale(b, "yaw-scale", 1.0),
          pitch_scale(b, "pitch-scale", 1.0),
          roll_scale(b, "roll-scale", 1.0),
          yaw_scale_immersive(b, "yaw-scale-immersive", 0.8),
          pitch_scale_immersive(b, "pitch-scale-immersive", 0.8),
          roll_scale_immersive(b, "roll-scale-immersive", 0.8)
    {}
};

class wxr_tracker : protected QThread, public ITracker
{
    Q_OBJECT
public:
    wxr_tracker();
    void Init();
    void ReceiveData();
    void KillReceiver();
    void SendData(std::string sendData);

    std::string GetRetData();
    ~wxr_tracker() override;
    module_status start_tracker(QFrame*) override;
    void data(double* data) override;

    QVector4D QuaternionMultiply(const QVector4D& q1, const QVector4D& q2);
    QVector3D HMDPos;
    QVector4D HMDRawQuat;
    bool isImmersive;
    bool isSBS; // Unused for now

private:
    int udpPort = 7872;
    int udpSocket;
    int udpSendPort = 7278;
    int udpSendSocket;
    std::thread udpReadThread;
    std::string retData;
    std::mutex mtx;
    std::condition_variable cv;
    settings s;

    double last[6]{};
    Timer t;
};

class wxr_dialog : public ITrackerDialog
{
    Q_OBJECT

public:
    wxr_dialog();
    void register_tracker(ITracker*) override {}
    void unregister_tracker() override {}

private:
    Ui::wxr_ui ui;
    settings s;

private slots:
    void doOK();
    void doCancel();
};

class wxr_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("WXR tracker"); }
    QIcon icon() override { return QIcon(":/images/opentrack.png"); }
};
