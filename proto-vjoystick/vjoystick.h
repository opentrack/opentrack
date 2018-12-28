/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */
#pragma once
#include "ui_vjoystick.h"
#include "api/plugin-api.hpp"
#include "compat/macros.hpp"

#include <windows.h>

enum state : signed char
{
    state_notent = -1,
    state_fail = -2,
    state_success = 1,
};

class handle final
{
public:
    static constexpr unsigned axis_count = 6;
    static const unsigned char axis_ids[axis_count];

private:
    state joy_state;
    LONG axis_min[6] {};
    LONG axis_max[6] {};

    void init();
public:
    handle();
    ~handle();
    state get_state() { return joy_state; }
    LONG to_axis_value(unsigned axis_id, double val);
};

class vjoystick_proto : public TR, public IProtocol
{
    Q_OBJECT

    handle h;
public:
    vjoystick_proto();
    ~vjoystick_proto() override;
    module_status initialize() override;
    void pose( const double *headpose ) override;
    QString game_name() override { return tr("Virtual joystick"); }
private:
};

class vjoystick_dialog final : public IProtocolDialog
{
    Q_OBJECT
public:

    vjoystick_dialog();
    void register_protocol(IProtocol *) override {}
    void unregister_protocol() override {}
private:
    Ui::vjoystick ui;
};

class vjoystick_metadata : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Joystick emulation -- vjoystick"); }
    QIcon icon() override { return QIcon(":/images/vjoystick.png"); }
};
