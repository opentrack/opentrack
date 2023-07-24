/* Copyright (c) 2023 Tom Brazier <tom_github@firstsolo.net>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include "ui_ftnoir_nm_filtercontrols.h"

#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "compat/hamilton-tools.h"
#include "options/options.hpp"

using namespace options;

struct settings_nm : opts
{
    value<slider_value> pos_responsiveness;
    value<slider_value> rot_responsiveness;
    value<slider_value> pos_drift_speed;
    value<slider_value> rot_drift_speed;

    settings_nm() :
        opts("nm-filter"),
        pos_responsiveness(value<slider_value>(b, "pos-responsiveness", { 13.0, .0, 20.0 })),
        rot_responsiveness(value<slider_value>(b, "rot-responsiveness", { 16.0, .0, 20.0 })),
        pos_drift_speed(value<slider_value>(b, "pos-drift-speed", { 5.0, 0.1, 50.0 })),
        rot_drift_speed(value<slider_value>(b, "rot-drift-speed", { 7.0, 0.1, 50.0 }))
    {
    }
};

struct filter_nm : IFilter
{
    filter_nm();
    void filter(const double* input, double* output) override;
    void center() override { first_run = true; }
    module_status initialize() override { return status_ok(); }

private:
    tVector last_pos_in;
    tQuat last_rot_in;
    tVector last_pos_out;
    tQuat last_rot_out;
    tVector last_pos_speed;
    tQuat last_rot_speed;
    Timer t;
    settings_nm s;
    bool first_run = true;
};

class dialog_nm : public IFilterDialog
{
    Q_OBJECT
public:
    dialog_nm();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
    void save() override;
    void reload() override;
    bool embeddable() noexcept override { return true; }
    void set_buttons_visible(bool x) override;

private:
    Ui::UICdialog_nm ui;
    settings_nm s;

private slots:
    void doOK();
    void doCancel();
};

class nmDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("NaturalMovement"); }
    QIcon icon() override { return QIcon(":/images/filter-16.png"); }
};
