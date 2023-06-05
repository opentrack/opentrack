/* Copyright (c) 2020, GO63-samara <go1@list.ru> 
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "api/plugin-api.hpp"
#include "ui_ftnoir_hamilton_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include "options/options.hpp"
//#include "compat/timer.hpp"
#include "compat/hamilton-tools.h"

using namespace options;

struct settings : opts {
    value<slider_value> kMaxRot,  kPowRot,  kDeadZoneRot,
                        kMaxDist, kPowDist, kDeadZoneDist,
                        kPowZoom, kMaxZ;
    settings() :
        opts         ("hamilton-filter"),
        kMaxRot      (b, "max-radius-smoothing",  { .01, .001,  25.0 }),
        kPowRot      (b, "smoothing-power-rot",   { .01, .001,   4.0 }),
        kDeadZoneRot (b, "dead-zone-radius-rot",  { .01, .001,   0.5 }),
        kMaxDist     (b, "max-distance-smoothing",{ .01, .001,  20.0 }),
        kPowDist     (b, "smoothing-power-dist",  { .01, .001,   4.0 }),
        kDeadZoneDist(b, "dead-zone-radius-dist", { .01, .001,   0.5 }),
        kPowZoom     (b, "smoothing-power-zoom",  { .01, .001,   4.0 }),
        kMaxZ        (b, "max-z",                 { .01, .001, 100.0 })
    {}
};

class hamilton : public IFilter
{
public:
    hamilton();
    void filter(const double *input, double *output) override;
    void center() override { first_run = true; }
    module_status initialize() override { return status_ok(); }
private:
    tQuat   quat_last;
    tVector pos_last;
    settings s;
    bool first_run = true;
};

class dialog_hamilton: public IFilterDialog
{
    Q_OBJECT
public:
    dialog_hamilton();
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}

private:
    Ui::UICdialog_hamilton ui;
    settings s;

private slots:
    void doOK();
    void doCancel();
};

class hamiltonDll : public Metadata
{
    Q_OBJECT

    QString name() { return tr("Hamilton"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};
