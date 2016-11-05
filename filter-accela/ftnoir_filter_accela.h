/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#pragma once
#include "accela-settings.hpp"
#include "api/plugin-api.hpp"
#include "compat/timer.hpp"
#include "ui_ftnoir_accela_filtercontrols.h"

#include <atomic>
#include <QMutex>
#include <QTimer>

class accela : public IFilter
{
public:
    accela();
    void filter(const double* input, double *output) override;
    void center() override { first_run = true; }
    spline rot, trans;
private:
    settings_accela s;
    bool first_run;
    double last_output[6];
    double smoothed_input[6];
    Timer t;

    template <typename T>
    static inline int signum(T x)
    {
        return (T(0) < x) - (x < T(0));
    }
};

class dialog_accela: public IFilterDialog
{
    Q_OBJECT
public:
    dialog_accela();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
private:
    Ui::AccelaUICdialog_accela ui;
    void save();
    settings_accela s;
private slots:
    void doOK();
    void doCancel();
    void update_ewma_display(const slider_value& value);
    void update_rot_display(const slider_value& value);
    void update_trans_display(const slider_value& value);
    void update_rot_dz_display(const slider_value& value);
    void update_trans_dz_display(const slider_value&);
    void update_rot_nl_slider(const slider_value& sl);
};

class accelaDll : public Metadata
{
public:
    QString name() { return QString(QCoreApplication::translate("accelaDll", "Accela")); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
