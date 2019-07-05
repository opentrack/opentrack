#include "module.hpp"

#include <libusb.h>

int device_count()
{
    libusb_context * ctx = nullptr;
    libusb_device **list = nullptr;
    ssize_t sz = 0;
    int rc = 0, cnt = 0;

    constexpr int vendor_id = 0x1415;
    constexpr int product_id = 0x2000;

    rc = libusb_init(&ctx);

    if (rc)
        goto end;

    sz = libusb_get_device_list(ctx, &list);

    if (sz < 0)
        goto end;

    for (int i = 0; i < sz; ++i) {
        libusb_device *device = list[i];
        libusb_device_descriptor desc = {};

        if (libusb_get_device_descriptor(device, &desc))
            goto end;

        if (desc.idVendor == vendor_id && desc.idProduct == product_id)
            cnt++;
    }

end:
    if (list)
        libusb_free_device_list(list, 1);
    if (ctx)
        libusb_exit(ctx);

    return cnt;
}

bool check_device_exists()
{
    static bool ret = device_count() > 0;
    return ret;
}

