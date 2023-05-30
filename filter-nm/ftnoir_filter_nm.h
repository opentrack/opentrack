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
#include "options/options.hpp"

using namespace options;

struct settings_nm : opts
{
    value<slider_value> responsiveness[6];
    value<slider_value> drift_speeds[6];

    settings_nm() :
        opts("nm-filter"),
          responsiveness{ value<slider_value>(b, "x-responsiveness",        { 10.0, .0, 20.0 }),
                          value<slider_value>(b, "y-responsiveness",        { 10.0, .0, 20.0 }),
                          value<slider_value>(b, "z-responsiveness",        { 10.0, .0, 20.0 }),
                          value<slider_value>(b, "yaw-responsiveness",      { 10.0, .0, 20.0 }),
                          value<slider_value>(b, "pitch-responsiveness",    { 10.0, .0, 20.0 }),
                          value<slider_value>(b, "roll-responsiveness",     { 10.0, .0, 20.0 }) },
          drift_speeds{ value<slider_value>(b, "x-drift-speed",     { 50.0, 1.0, 200.0 }),
                        value<slider_value>(b, "y-drift-speed",     { 50.0, 1.0, 200.0 }),
                        value<slider_value>(b, "z-drift-speed",     { 50.0, 1.0, 200.0 }),
                        value<slider_value>(b, "yaw-drift-speed",   { 100.0, 1.0, 400.0 }),
                        value<slider_value>(b, "pitch-drift-speed", { 100.0, 1.0, 400.0 }),
                        value<slider_value>(b, "roll-drift-speed",  { 100.0, 1.0, 400.0 }) }
    {
    }

    /*    value<slider_value> kMinSmoothing, kMaxSmoothing, kSmoothingScaleCurve;
    settings()
        : opts("ewma-filter"),
          kMinSmoothing(b, "min-smoothing", { .02, .01, 1 }),
          kMaxSmoothing(b, "max-smoothing", { .7, .01, 1 }),
          kSmoothingScaleCurve(b, "smoothing-scale-curve", { .8, .1, 5 })
    {
    }
*/
};

struct filter_nm : IFilter
{
    filter_nm();
    void filter(const double* input, double* output) override;
    void center() override { first_run = true; }
    module_status initialize() override { return status_ok(); }

private:
    double last_input[6]{};
    double speeds[6]{};
    double filtered_output[6]{};
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
