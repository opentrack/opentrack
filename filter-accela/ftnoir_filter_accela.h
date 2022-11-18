/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once

#include "ui_ftnoir_accela_filtercontrols.h"

#include "accela-settings.hpp"
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "compat/variance.hpp"

#include <QMutex>
#include <QTimer>

//#define DEBUG_ACCELA

struct accela : IFilter
{
    accela();
    void filter(const double* input, double *output) override;
    void center() override { first_run = true; }
    spline spline_rot, spline_pos;
    module_status initialize() override { return status_ok(); }
private:
    settings_accela s;
    double last_output[6] {}, deltas[6] {};
    Timer t;
#if defined DEBUG_ACCELA
    Timer debug_timer;
    double debug_max;
    variance var;
#endif
    bool first_run = true;
};

class dialog_accela : public IFilterDialog
{
    Q_OBJECT
public:
    dialog_accela();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
    void save() override;
    void reload() override;
    bool embeddable() noexcept override { return true; }
    void set_buttons_visible(bool x) override;
private:
    Ui::AccelaUICdialog_accela ui;
    settings_accela s;
private slots:
    void doOK();
    void doCancel();
};

class accelaDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Accela"); }
    QIcon icon() override { return QIcon(":/images/filter-16.png"); }
};
