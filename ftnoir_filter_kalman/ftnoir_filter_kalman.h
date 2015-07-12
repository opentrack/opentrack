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
    // slider goes noise_stdder 0->50
    static constexpr double mult_noise_stddev = .5;
    settings() : opts("kalman-filter"), noise_stddev_slider(b, "noise-stddev", 10)
    {}
};

class OPENTRACK_EXPORT FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void reset();
    void filter(const double *input, double *output);
    // Set accel_stddev assuming moving 0.0->100.0 in dt=0.2 is 3 stddevs: (100.0*4/dt^2)/3.
    const double accel_stddev = (100.0*4/(0.2*0.2))/3.0;
    // TODO(abo): make noise_stddev a UI setting 0.0->10.0 with 0.1 resolution.
    //const double noise_stddev = 1.0;
    cv::KalmanFilter kalman;
    double last_input[6];
    QElapsedTimer timer;
    settings s;
    int prev_slider_pos;
};

class OPENTRACK_EXPORT FTNoIR_FilterDll : public Metadata
{
public:
    QString name() { return QString("Kalman"); }
   QIcon icon() { return QIcon(":/images/filter-16.png"); }
};

class OPENTRACK_EXPORT FilterControls: public IFilterDialog
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
