/* Copyright (c) 2012-2015 Stanislaw Halik
 * Copyright (c) 2023-2024 Michael Welter
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include "ui_accela_hamilton_filtercontrols.h"

#include "accela_hamilton_settings.hpp"
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "compat/variance.hpp"
#include "compat/hamilton-tools.h"

#include <QMutex>
#include <QTimer>


//#define DEBUG_ACCELA

struct accela_hamilton : IFilter
{
    accela_hamilton();
    void filter(const double* input, double *output) override;
    void center() override { first_run = true; }
    spline spline_rot, spline_pos;
    module_status initialize() override { return status_ok(); }
private:
    settings_accela_hamilton s;
    tVector last_position = {};
    tQuat last_rotation = {};
    Timer t;
#if defined DEBUG_ACCELA
    Timer debug_timer;
    double debug_max;
    variance var;
#endif
    bool first_run = true;
};

class dialog_accela_hamilton : public IFilterDialog
{
    Q_OBJECT
public:
    dialog_accela_hamilton();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
    void save() override;
    void reload() override;
    bool embeddable() noexcept override { return true; }
    void set_buttons_visible(bool x) override;
private:
    Ui::AccelaUICdialog_accela ui;
    settings_accela_hamilton s;
private slots:
    void doOK();
    void doCancel();
};

class accela_hamiltonDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("AccelaHamilton"); }
    QIcon icon() override { return QIcon(":/images/filter-16.png"); }
};
