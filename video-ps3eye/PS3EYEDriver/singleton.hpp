#pragma once

#include <vector>
#include <memory>
#include <thread>
#include <atomic>

struct libusb_context;
struct ps3eye_camera;

struct USBMgr
{
    USBMgr();
    ~USBMgr();
    USBMgr(const USBMgr&) = delete;
    void operator=(const USBMgr&) = delete;

    static USBMgr& instance();
    int list_devices(std::vector<std::shared_ptr<ps3eye_camera>>& list);
    void camera_started();
    void camera_stopped();

private:
    libusb_context* usb_context = nullptr;
    std::thread update_thread;
    std::atomic_int active_camera_count = 0;
    std::atomic_bool exit_signaled = false;

    void start_xfer_thread();
    void stop_xfer_thread();
    void transfer_loop();
};
