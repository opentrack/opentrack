/* Copyright (c) 2017, Eike Ziller                                               *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#pragma once

#include <QByteArray>
#include <QString>

#include <IOKit/IOKitLib.h>

struct JoystickValues {
    uint8_t x;
    uint8_t y;
    uint8_t z;
    uint8_t rx;
    uint8_t ry;
    uint8_t rz;
};

class FooHIDJoystick
{
public:
    FooHIDJoystick(const QByteArray &name, const QByteArray &serialNumber);
    ~FooHIDJoystick();

    bool hasError() const;
    QString errorMessage() const;

    int minValue() const { return 0; }
    int range() const { return 255; }

    void setValue(JoystickValues newValues);

private:
    bool createDevice() const;
    bool sendToDevice() const;
    void destroyDevice() const;

    const QByteArray name;
    const QByteArray serialNumber;
    io_connect_t connection = 0;
    JoystickValues values = {127, 127};
    QString _errorMessage;
    bool _hasError = true;
    bool connectionOpened = false;
    bool deviceCreated = false;
};
