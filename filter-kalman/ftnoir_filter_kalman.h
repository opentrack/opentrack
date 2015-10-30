#pragma once
/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#ifndef INCLUDED_FTN_FILTER_H
#define INCLUDED_FTN_FILTER_H

#include "ui_ftnoir_kalman_filtercontrols.h"
#include "opentrack/plugin-api.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/video/video.hpp>
#include <vector>
#include <QString>
#include <QElapsedTimer>
#include <QWidget>
#include "opentrack/options.hpp"
using namespace options;

struct settings : opts {
    value<int> noise_stddev_slider;
    // slider for noise_stddev goes 0->(mult_noise_stddev * 100)
    static constexpr double mult_noise_stddev = .5;
    settings() : opts("kalman-filter"), noise_stddev_slider(b, "noise-stddev", 40)
    {}
};

class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void reset();
    void filter(const double *input, double *output);
    // Set accel_stddev assuming moving 0.0->accel in dt_ is 3 stddevs: (accel*4/dt_^2)/3.
    static constexpr double dt_ = .4;
    static constexpr double accel = 60.;
    static constexpr double accel_stddev = (accel*4/(dt_*dt_))/3.0;
    cv::KalmanFilter kalman;
    double last_input[6];
    QElapsedTimer timer;
    settings s;
    int prev_slider_pos;
};

class FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Kalman"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};

class FilterControls: public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls() {
        ui.setupUi(this);
        connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
        connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
        tie_setting(s.noise_stddev_slider, ui.noise_slider);
    }
    Ui::KalmanUICFilterControls ui;
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
    settings s;
public slots:
    void doOK();
    void doCancel();
};

#endif
