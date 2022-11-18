/* Copyright (c) 2016, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */
#pragma once
#include "ui_vjoystick.h"
#include "api/plugin-api.hpp"

enum status
{
};

class vjoystick : public TR, public IProtocol
{
    Q_OBJECT

public:
    vjoystick();
    ~vjoystick() override;
    module_status initialize() override;
    void pose(const double* headpose, const double*) override;
    QString game_name() override { return tr("Virtual joystick"); }

private:
    long axis_min[6] {};
    long axis_max[6] {};
    [[nodiscard]] bool init();
    int to_axis_value(unsigned axis_id, double val) const;

    static constexpr unsigned axis_count = 6;
    static const unsigned char axis_ids[axis_count];

    bool status = false;
    bool first_run = true;
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
