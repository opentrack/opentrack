/* Copyright (c) 2017, Eike Ziller                                               *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#include "iokitprotocol.h"

#include "foohidjoystick.h"
#include "iokitprotocoldialog.h"

#include <QDebug>

IOKitProtocol::IOKitProtocol()
{
    joystick = std::make_unique<FooHIDJoystick>("opentrack Virtual Joystick",
                                                "SN 983457");
    if (joystick->hasError())
        qWarning("%s\n", qPrintable(joystick->errorMessage()));
}

module_status IOKitProtocol::initialize()
{
    if (!joystick)
        return tr("Load failure");

    if (joystick->hasError())
    {
        QString msg = joystick->errorMessage();

        if (msg.isEmpty())
            msg = tr("Unknown error");

        return error(msg);
    }

    return status_ok();
}

static uint8_t valueToStick(FooHIDJoystick *stick, double min, double value, double max)
{
    return uint8_t(qBound(stick->minValue(),
                   int(round((value - min) * stick->range() / (max - min) - stick->minValue())),
                   stick->minValue() + stick->range()));
}

void IOKitProtocol::pose(const double *headpose, const double*)
{
    const uint8_t x  = valueToStick(&*joystick, -75., headpose[0], +75.);
    const uint8_t y  = valueToStick(&*joystick, -75., headpose[1], +75.);
    const uint8_t z  = valueToStick(&*joystick, -75., headpose[2], +75.);
    const uint8_t rx = valueToStick(&*joystick, -180., headpose[3], +180.);
    const uint8_t ry = valueToStick(&*joystick, -180., headpose[4], +180.);
    const uint8_t rz = valueToStick(&*joystick, -180., headpose[5], +180.);
    joystick->setValue({x, y, z, rx, ry, rz});
}

QString IOKitProtocol::game_name()
{
    return QString();
}

OPENTRACK_DECLARE_PROTOCOL(IOKitProtocol, IOKitProtocolDialog, IOKitProtocolMetadata)
