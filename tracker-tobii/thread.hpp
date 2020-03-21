#pragma once

#include <QThread>
#include <QCoreApplication>

#include <tobii/tobii.h>
#include <tobii/tobii_streams.h>
#include <atomic>
#include <vector>
#include <string>

class tobii_thread : public QThread
{
    Q_OBJECT
    void run() override;

public:
    tobii_thread()
    {
        head_pose = new tobii_head_pose_t();
    }
    ~tobii_thread() override;

    tobii_head_pose_t* head_pose;

private:
    tobii_api_t* api;
    tobii_device_t* device;
    
    static constexpr unsigned int retries = 300;
    static constexpr unsigned int interval = 100;

    QString error_last = "";
    std::atomic<bool> exit_thread = false;
};
