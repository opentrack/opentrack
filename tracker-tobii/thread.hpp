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

signals:
    void tobii_error_signal(QString error_message);
    void tobii_ready_signal();

public:
    tobii_thread();
    ~tobii_thread() override;

    tobii_head_pose_t* head_pose;

private:
    tobii_api_t* api;
    tobii_device_t* device;
    
    const unsigned int retries = 300;
    const unsigned int interval = 100;
    std::atomic<bool> exit_thread = false;

    void tobii_error_signal_impl(QString error_message);
    void tobii_ready_signal_impl();
};
