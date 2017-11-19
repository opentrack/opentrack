/* Copyright (c) 2017, Eike Ziller                                               *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#pragma once

#include "api/plugin-api.hpp"

#include <QCoreApplication>

class FooHIDJoystick;

class IOKitProtocol : public IProtocol
{
public:
    IOKitProtocol();

    bool correct() final;
    void pose(const double *headpose) final;
    QString game_name() final;

private:
    std::unique_ptr<FooHIDJoystick> joystick;
};

class IOKitProtocolMetadata : public Metadata
{
public:
    QString name() { return QString(QCoreApplication::translate("iokit-foohid", "Virtual joystick")); }
    QIcon icon() { return QIcon(":/images/facetracknoir.png"); }
};
