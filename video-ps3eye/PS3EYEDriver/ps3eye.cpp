#include "ps3eye.hpp"
#include "compat/macros1.h"
#include "compat/thread-name.hpp"
#include "internal.hpp"

#include "urb.hpp"
#include "frame-queue.hpp"
#include "singleton.hpp"
#include "constants.hpp"

#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <atomic>
#include <optional>
#include <iterator>
#include <algorithm>

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cstdint>

using namespace std::chrono_literals;

#define OV534_REG_ADDRESS       0xf1    /* sensor address */
#define OV534_REG_SUBADDR       0xf2
#define OV534_REG_WRITE         0xf3
#define OV534_REG_READ          0xf4
#define OV534_REG_OPERATION     0xf5
#define OV534_REG_STATUS        0xf6

#define OV534_OP_WRITE_3        0x37
#define OV534_OP_WRITE_2        0x33
#define OV534_OP_READ_2         0xf9

bool ps3eye_camera::devicesEnumerated = false;
std::vector<ps3eye_camera::device> ps3eye_camera::devices;

const std::vector<ps3eye_camera::device>& ps3eye_camera::get_devices(bool force_refresh)
{
    if (devicesEnumerated && !force_refresh)
        return devices;

    devices.clear();

    (void) USBMgr::instance().list_devices(devices);

    devicesEnumerated = true;
    return devices;
}

ps3eye_camera::ps3eye_camera(libusb_device *device) :
    device_(device),
    urb(std::make_shared<URBDesc>())
{
}

ps3eye_camera::~ps3eye_camera()
{
    release();
}

void ps3eye_camera::release()
{
    stop();
    close_usb();
}

bool ps3eye_camera::init(uint32_t width, uint32_t height, uint16_t desiredFrameRate, format outputFormat)
{
    uint16_t sensor_id;

    // open usb device so we can setup and go
    if(handle_ == nullptr && !open_usb())
        return false;

    // find best cam mode
    if((width == 0 && height == 0) || width > 320 || height > 240)
    {
        frame_width = 640;
        frame_height = 480;
    } else {
        frame_width = 320;
        frame_height = 240;
    }
    frame_rate = ov534_set_frame_rate(desiredFrameRate, true);
    frame_output_format = outputFormat;

    /* reset bridge */
    ov534_reg_write(0xe7, 0x3a);
    ov534_reg_write(0xe0, 0x08);

    std::this_thread::sleep_for(100ms);

    /* initialize the sensor address */
    ov534_reg_write(OV534_REG_ADDRESS, 0x42);

    /* reset sensor */
    sccb_reg_write(0x12, 0x80);

    std::this_thread::sleep_for(100ms);

    /* probe the sensor */
    sccb_reg_read(0x0a);
    sensor_id = (uint16_t)(sccb_reg_read(0x0a) << 8);
    sccb_reg_read(0x0b);
    sensor_id |= sccb_reg_read(0x0b);
    debug("sensor id: %04x\n", sensor_id);

    /* initialize */
    reg_w_array(ov534_reg_initdata, std::size(ov534_reg_initdata));
    ov534_set_led(1);
    sccb_w_array(ov772x_reg_initdata, std::size(ov772x_reg_initdata));
    ov534_reg_write(0xe0, 0x09);
    ov534_set_led(0);

    return true;
}

bool ps3eye_camera::start()
{
    if (is_streaming_)
        return true;

    if (!isInitialized())
    {
        debug("call ps3eye_camera::init() first\n");
        return false;
    }

    if (frame_width == 320) {       /* 320x240 */
        reg_w_array(bridge_start_qvga, std::size(bridge_start_qvga));
        sccb_w_array(sensor_start_qvga, std::size(sensor_start_qvga));
    } else {                /* 640x480 */
        reg_w_array(bridge_start_vga, std::size(bridge_start_vga));
        sccb_w_array(sensor_start_vga, std::size(sensor_start_vga));
    }

    ov534_set_frame_rate(frame_rate);

    set_auto_gain(autogain);
    set_auto_white_balance(awb);
    set_gain(gain);
    set_hue(hue);
    set_exposure(exposure);
    set_brightness(brightness);
    set_contrast(contrast);
    set_sharpness(sharpness);
    set_red_balance(redblc);
    set_blue_balance(blueblc);
    set_green_balance(greenblc);
    set_flip(flip_h, flip_v);

    ov534_set_led(1);
    ov534_reg_write(0xe0, 0x00); // start stream

    is_streaming_ = true;
    // init and start urb
    if (!urb->start_transfers(handle_, frame_width*frame_height))
    {
        debug("failed to start\n");
        stop();
        return false;
    }
    return true;
}

void ps3eye_camera::stop()
{
    if (!is_streaming_)
        return;

    /* stop streaming data */
    ov534_reg_write(0xe0, 0x09);
    ov534_set_led(0);

    // close urb
    urb->close_transfers();
    urb->free_transfers();

    is_streaming_ = false;
}

#define MAX_USB_DEVICE_PORT_PATH 7

bool ps3eye_camera::getUSBPortPath(char *out_identifier, size_t max_identifier_length) const
{
    bool success = false;

    if (isInitialized())
    {
        uint8_t port_numbers[MAX_USB_DEVICE_PORT_PATH];

        memset(out_identifier, 0, max_identifier_length);
        memset(port_numbers, 0, std::size(port_numbers));

        int port_count = libusb_get_port_numbers(device_, port_numbers, MAX_USB_DEVICE_PORT_PATH);
        int bus_id = libusb_get_bus_number(device_);

        snprintf(out_identifier, max_identifier_length, "b%d", bus_id);
        if (port_count > 0)
        {
            success = true;

            for (int port_index = 0; port_index < port_count; ++port_index)
            {
                uint8_t port_number = port_numbers[port_index];
                char port_string[8];

                snprintf(port_string, std::size(port_string), (port_index == 0) ? "_p%d" : ".%d", port_number);

                if (strlen(out_identifier)+std::size(port_string)+1 <= max_identifier_length)
                    std::strcat(out_identifier, port_string);
                else
                {
                    success = false;
                    break;
                }
            }
        }
    }

    return success;
}

uint32_t ps3eye_camera::get_depth() const
{
    switch (frame_output_format)
    {
    case format_Bayer:
    case format_Gray:
        return 1;
    case format_BGR:
    case format_RGB:
        return 3;
    case format_BGRA:
    case format_RGBA:
        return 4;
    default:
        unreachable();
    }
}

bool ps3eye_camera::get_frame(uint8_t* frame)
{
    return urb->queue().Dequeue(frame, frame_width, frame_height, frame_output_format, flip_v);
}

bool ps3eye_camera::open_usb()
{
    // open, set first config and claim interface
    int res = libusb_open(device_, &handle_);
    if(res != 0) {
        debug("device open error: %d\n", res);
        return false;
    }

    res = libusb_claim_interface(handle_, 0);
    if(res != 0) {
        debug("device claim interface error: %d\n", res);
        return false;
    }

    return true;
}

void ps3eye_camera::close_usb()
{
    if (handle_)
    {
        libusb_release_interface(handle_, 0);
        libusb_close(handle_);
    }
    if (device_)
        libusb_unref_device(device_);

    handle_ = nullptr;
    device_ = nullptr;
}

/* Two bits control LED: 0x21 bit 7 and 0x23 bit 7.
 * (direction and output)? */
void ps3eye_camera::ov534_set_led(int status)
{
    uint8_t data;

    debug("led status: %d\n", status);

    data = ov534_reg_read(0x21);
    data |= 0x80;
    ov534_reg_write(0x21, data);

    data = ov534_reg_read(0x23);
    if (status)
        data |= 0x80;
    else
        data &= ~0x80;

    ov534_reg_write(0x23, data);

    if (!status) {
        data = ov534_reg_read(0x21);
        data &= ~0x80;
        ov534_reg_write(0x21, data);
    }
}

/* validate frame rate and (if not dry run) set it */
uint16_t ps3eye_camera::ov534_set_frame_rate(uint16_t fps, bool dry_run)
{
    const struct rate_s *r;
    int i;

    if (frame_width == 640) {
        r = rate_0;
        i = std::size(rate_0);
    } else {
        r = rate_1;
        i = std::size(rate_1);
    }
    while (--i > 0) {
        if (fps >= r->fps)
            break;
        r++;
    }

    if (!dry_run) {
        sccb_reg_write(0x11, r->r11);
        sccb_reg_write(0x0d, r->r0d);
        ov534_reg_write(0xe5, r->re5);
    }

    debug("frame_rate: %d\n", r->fps);
    return r->fps;
}

void ps3eye_camera::ov534_reg_write(uint16_t reg, uint8_t val)
{
    int ret;

    //debug("reg=0x%04x, val=0%02x", reg, val);
    usb_buf[0] = val;

    constexpr int req_type = (int)LIBUSB_ENDPOINT_OUT |
                             (int)LIBUSB_REQUEST_TYPE_VENDOR |
                             (int)LIBUSB_RECIPIENT_DEVICE;

    ret = libusb_control_transfer(handle_,
                                  req_type,
                                  0x01, 0x00, reg,
                                  usb_buf, 1, 500);
    if (ret < 0) {
        debug("write failed\n");
    }
}

uint8_t ps3eye_camera::ov534_reg_read(uint16_t reg)
{
    int ret;

    constexpr int req_type = (int)LIBUSB_ENDPOINT_IN |
                             (int)LIBUSB_REQUEST_TYPE_VENDOR |
                             (int)LIBUSB_RECIPIENT_DEVICE;

    ret = libusb_control_transfer(handle_, req_type, 0x01, 0x00, reg, usb_buf, 1, 500);

    //debug("reg=0x%04x, data=0x%02x", reg, usb_buf[0]);
    if (ret < 0)
        warn("read failed\n");
    return usb_buf[0];
}

int ps3eye_camera::sccb_check_status()
{
    uint8_t data;
    int i;

    for (i = 0; i < 5; i++) {
        data = ov534_reg_read(OV534_REG_STATUS);

        switch (data) {
        case 0x00:
            return 1;
        case 0x04:
            return 0;
        case 0x03:
            break;
        default:
            debug("sccb status 0x%02x, attempt %d/5\n",
                  data, i + 1);
        }
    }
    return 0;
}

void ps3eye_camera::sccb_reg_write(uint8_t reg, uint8_t val)
{
    //debug("reg: 0x%02x, val: 0x%02x", reg, val);
    ov534_reg_write(OV534_REG_SUBADDR, reg);
    ov534_reg_write(OV534_REG_WRITE, val);
    ov534_reg_write(OV534_REG_OPERATION, OV534_OP_WRITE_3);

    if (!sccb_check_status()) {
        debug("sccb_reg_write failed\n");
    }
}


uint8_t ps3eye_camera::sccb_reg_read(uint16_t reg)
{
    ov534_reg_write(OV534_REG_SUBADDR, (uint8_t)reg);
    ov534_reg_write(OV534_REG_OPERATION, OV534_OP_WRITE_2);
    if (!sccb_check_status()) {
        debug("sccb_reg_read failed 1\n");
    }

    ov534_reg_write(OV534_REG_OPERATION, OV534_OP_READ_2);
    if (!sccb_check_status()) {
        debug( "sccb_reg_read failed 2\n");
    }

    return ov534_reg_read(OV534_REG_READ);
}
/* output a bridge sequence (reg - val) */
void ps3eye_camera::reg_w_array(const uint8_t (*data)[2], int len)
{
    while (--len >= 0) {
        ov534_reg_write((*data)[0], (*data)[1]);
        data++;
    }
}

/* output a sensor sequence (reg - val) */
void ps3eye_camera::sccb_w_array(const uint8_t (*data)[2], int len)
{
    while (--len >= 0) {
        if ((*data)[0] != 0xff) {
            sccb_reg_write((*data)[0], (*data)[1]);
        } else {
            sccb_reg_read((*data)[1]);
            sccb_reg_write(0xff, 0x00);
        }
        data++;
    }
}

void ps3eye_camera::set_auto_white_balance(bool val)
{
    awb = val;
    if (val) {
        sccb_reg_write(0x63, 0xe0); //AWB ON
    }else{
        sccb_reg_write(0x63, 0xAA); //AWB OFF
    }
}

void ps3eye_camera::set_gain(uint8_t val)
{
    gain = val;
    switch(val & 0x30){
    case 0x00:
        val &=0x0F;
        break;
    case 0x10:
        val &=0x0F;
        val |=0x30;
        break;
    case 0x20:
        val &=0x0F;
        val |=0x70;
        break;
    case 0x30:
        val &=0x0F;
        val |=0xF0;
        break;
    }
    sccb_reg_write(0x00, val);
}

void ps3eye_camera::set_exposure(uint8_t val)
{
    exposure = val;
    sccb_reg_write(0x08, val>>7);
    sccb_reg_write(0x10, val<<1);
}

void ps3eye_camera::set_sharpness(uint8_t val)
{
    sharpness = val;
    sccb_reg_write(0x91, val); //vga noise
    sccb_reg_write(0x8E, val); //qvga noise
}

void ps3eye_camera::set_contrast(uint8_t val)
{
    contrast = val;
    sccb_reg_write(0x9C, val);
}

void ps3eye_camera::set_brightness(uint8_t val)
{
    brightness = val;
    sccb_reg_write(0x9B, val);
}

void ps3eye_camera::set_hue(uint8_t val)
{
    hue = val;
    sccb_reg_write(0x01, val);
}

void ps3eye_camera::set_red_balance(uint8_t val)
{
    redblc = val;
    sccb_reg_write(0x43, val);
}

void ps3eye_camera::set_blue_balance(uint8_t val)
{
    blueblc = val;
    sccb_reg_write(0x42, val);
}

void ps3eye_camera::set_green_balance(uint8_t val)
{
    greenblc = val;
    sccb_reg_write(0x44, val);
}

void ps3eye_camera::set_flip(bool horizontal, bool vertical)
{
    flip_h = horizontal;
    flip_v = vertical;
    uint8_t val = sccb_reg_read(0x0c);
    val &= ~0xc0;
    if (!horizontal) val |= 0x40;
    if (!vertical) val |= 0x80;
    sccb_reg_write(0x0c, val);
}

void ps3eye_camera::set_test_pattern(bool enable)
{
    testPattern = enable;
    uint8_t val = sccb_reg_read(0x0C);
    val &= ~0b00000001;
    if (testPattern) val |= 0b00000001; // 0x80;
    sccb_reg_write(0x0C, val);
}

bool ps3eye_camera::setFrameRate(uint8_t val)
{
    if (is_streaming_) return false;
    frame_rate = ov534_set_frame_rate(val, true);
    return true;
}

void ps3eye_camera::set_auto_gain(bool val)
{
    autogain = val;
    if (val) {
        sccb_reg_write(0x13, 0xf7); //AGC,AEC,AWB ON
        sccb_reg_write(0x64, sccb_reg_read(0x64)|0x03);
    } else {
        sccb_reg_write(0x13, 0xf0); //AGC,AEC,AWB OFF
        sccb_reg_write(0x64, sccb_reg_read(0x64)&0xFC);

        set_gain(gain);
        set_exposure(exposure);
    }
}
