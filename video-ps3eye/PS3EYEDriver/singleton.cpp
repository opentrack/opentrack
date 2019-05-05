#include "singleton.hpp"
#include "internal.hpp"
#include "ps3eye.hpp"

//#define USE_CONTEXT

USBMgr::USBMgr()
{
#ifdef USE_CONTEXT
    libusb_init(&usb_context);
#else
    libusb_init(nullptr);
#endif
    libusb_set_debug(usb_context, 3);
}

USBMgr::~USBMgr()
{
#ifdef USE_CONTEXT
    stop_xfer_thread();
    libusb_exit(usb_context);
#endif
}

USBMgr& USBMgr::instance()
{
    static USBMgr ret;
    return ret;
}

void USBMgr::camera_started()
{
    if (++active_camera_count == 1)
        start_xfer_thread();
}

void USBMgr::camera_stopped()
{
    if (--active_camera_count == 0)
        stop_xfer_thread();
}

void USBMgr::start_xfer_thread()
{
    update_thread = std::thread(&USBMgr::transfer_loop, this);
}

void USBMgr::stop_xfer_thread()
{
    exit_signaled = true;
    update_thread.join();
    // Reset the exit signal flag.
    // If we don't and we call start_xfer_thread() again, transfer_loop will exit immediately.
    exit_signaled = false;
}

void USBMgr::transfer_loop()
{
    struct timeval tv {
        0,
        50 * 1000, // ms
    };

    debug2("-> xfer loop");

    while (!exit_signaled.load(std::memory_order_relaxed))
    {
        int status = libusb_handle_events_timeout_completed(usb_context, &tv, nullptr);

        if (status != LIBUSB_SUCCESS && status != LIBUSB_ERROR_INTERRUPTED)
            debug("libusb error(%d) %s\n", status, libusb_strerror((enum libusb_error)status));
    }
    debug2("<- xfer loop\n");
}

int USBMgr::list_devices(std::vector<ps3eye_camera::device>& list)
{
    libusb_device **devs;
    libusb_device_handle *devhandle;
    int cnt = (int)libusb_get_device_list(usb_context, &devs);

    if (cnt < 0)
        debug("device scan failed\n");

    cnt = 0;
    for (int i = 0; devs[i]; i++)
    {
        libusb_device* dev = devs[i];
        struct libusb_device_descriptor desc;

        if (libusb_get_device_descriptor(dev, &desc))
            continue;
        if (desc.idVendor == ps3eye_camera::VENDOR_ID && desc.idProduct == ps3eye_camera::PRODUCT_ID)
        {
            if (libusb_open(dev, &devhandle))
                continue;
            libusb_ref_device(dev);
            libusb_close(devhandle);
            list.emplace_back(std::make_shared<ps3eye_camera>(dev));
            cnt++;
        }
    }

    libusb_free_device_list(devs, 1);

    return cnt;
}
