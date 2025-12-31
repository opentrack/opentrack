#pragma once
#pragma comment(lib, "ws2_32.lib")
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "ui_wxr.h"

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

#include <QMatrix4x4>
#include <QQuaternion>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>

class wxr_tracker : public ITracker
{
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
    QMatrix4x4 RotationFromDirection(QVector3D direction);
    QVector3D HMDPos;
    QVector4D HMDQuat;

private:
    int udpPort = 7872;
    int udpSocket;
    int udpSendPort = 7278;
    int udpSendSocket;
    std::thread udpReadThread;
    std::string retData;
    std::mutex mtx;
    std::condition_variable cv;

    double last[6]{};
    Timer t;
};

class wxr_dialog : public ITrackerDialog
{
    Q_OBJECT

    Ui::wxr_ui ui;

public:
    wxr_dialog();
    void register_tracker(ITracker*) override {}
    void unregister_tracker() override {}
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
