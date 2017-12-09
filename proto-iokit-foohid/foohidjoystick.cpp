/* Copyright (c) 2017, Eike Ziller                                               *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#include "foohidjoystick.h"

#include "compat/macros.hpp"

const char FOOHID_SERVICE_NAME[] = "it_unbit_foohid";

enum class FooHIDMethod {
    Create = 0,
    Destroy,
    Send,
    List,
    Subscribe
};

// Joystick USB descriptor
static unsigned char report_descriptor[] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x04,                    // USAGE (Joystick)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x09, 0x32,                    //     USAGE (Z)
    0x09, 0x33,                    //     USAGE (RX)
    0x09, 0x34,                    //     USAGE (RY)
    0x09, 0x35,                    //     USAGE (RZ)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x26, 0xff, 0x00,              //     LOGICAL_MAXIMUM (255)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x06,                    //     REPORT_COUNT (6)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

static bool connectToService(io_connect_t *connection, QString *errorMessage)
{
    io_iterator_t iterator;
    io_service_t service;
    // Get an iterator over all IOService instances
    kern_return_t ret = IOServiceGetMatchingServices(kIOMasterPortDefault,
                                                     IOServiceMatching(FOOHID_SERVICE_NAME),
                                                     &iterator);
    if (ret != KERN_SUCCESS) {
        *errorMessage = otr_tr("Unable to find FooHID IOService.");
        return false;
    }
    // Iterate over services and try to open connection
    bool found = false;
    while ((service = IOIteratorNext(iterator)) != IO_OBJECT_NULL) {
        ret = IOServiceOpen(service, mach_task_self(), 0, connection);
        if (ret == KERN_SUCCESS) {
            found = true;
            break;
        }
        IOObjectRelease(service);
    }
    IOObjectRelease(iterator);
    if (!found) {
        *errorMessage = otr_tr("Unable to connect to FooHID IOService.");
        return false;
    }
    return true;
}

static void disconnectFromService(io_connect_t connection)
{
    IOServiceClose(connection);
    // ignore errors
}

FooHIDJoystick::FooHIDJoystick(const QByteArray &name, const QByteArray &serialNumber)
    : name(name), serialNumber(serialNumber)
{
    connectionOpened = connectToService(&connection, &_errorMessage);
    _hasError = !connectionOpened;
    if (!_hasError) {
        // first try to destroy device, in case it was left-over (from a crash/interrupt)
        destroyDevice();
        deviceCreated = createDevice();
        _hasError = !deviceCreated;
        if (!deviceCreated)
            _errorMessage = otr_tr("Failed to create virtual joystick");
    }
}

FooHIDJoystick::~FooHIDJoystick()
{
    if (deviceCreated)
        destroyDevice();
    if (connectionOpened)
        disconnectFromService(connection);
}

bool FooHIDJoystick::hasError() const
{
    return _hasError;
}

QString FooHIDJoystick::errorMessage() const
{
    return _errorMessage;
}

void FooHIDJoystick::setValue(JoystickValues newValues)
{
    values = newValues;
    if (!sendToDevice()) {
        _hasError = true;
        _errorMessage = otr_tr("Failed to send values to virtual joystick");
    }
}

bool FooHIDJoystick::createDevice() const
{
    uint64_t params[8];
    params[0] = uint64_t(name.constData());         // pointer to name
    params[1] = uint64_t(name.size());              // size of name without \0
    params[2] = uint64_t(report_descriptor);        // pointer to report descriptor
    params[3] = sizeof(report_descriptor);          // size of report descriptor
    params[4] = uint64_t(serialNumber.constData()); // pointer to serial number
    params[5] = uint64_t(serialNumber.size());      // size of serial number without \0
    params[6] = uint64_t(2);                        // vendor ID
    params[7] = uint64_t(3);                        // device ID

    kern_return_t ret = IOConnectCallScalarMethod(connection, int(FooHIDMethod::Create), params,
                                                  sizeof(params)/sizeof(params[0]), NULL, 0);
    return ret == KERN_SUCCESS;
}

bool FooHIDJoystick::sendToDevice() const
{
    uint64_t params[4];
    params[0] = uint64_t(name.constData());        // pointer to name
    params[1] = uint64_t(name.size());             // size of name without \0
    params[2] = uint64_t(&values);                 // pointer to values
    params[3] = sizeof(struct JoystickValues);     // length of value struct
    kern_return_t ret = IOConnectCallScalarMethod(connection, int(FooHIDMethod::Send), params,
                                                  sizeof(params)/sizeof(params[0]), NULL, 0);
    return ret == KERN_SUCCESS;
}

void FooHIDJoystick::destroyDevice() const
{
    uint64_t params[2];
    params[0] = uint64_t(name.constData()); // pointer to name
    params[1] = uint64_t(name.size());      // size of name without \0
    IOConnectCallScalarMethod(connection, int(FooHIDMethod::Destroy), params,
                              sizeof(params)/sizeof(params[0]), NULL, 0);
    // ignore failure
}
