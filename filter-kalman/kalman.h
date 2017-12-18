#pragma once
/* Copyright (c) 2016 Michael Welter <mw.pub@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ui_ftnoir_kalman_filtercontrols.h"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;
#include "compat/timer.hpp"

#include <Eigen/Core>
#include <Eigen/LU>

#include <QString>
#include <QWidget>

#include <atomic>

static constexpr int NUM_STATE_DOF = 12;
static constexpr int NUM_MEASUREMENT_DOF = 6;
// These vectors are compile time fixed size, stack allocated
using StateToMeasureMatrix = Eigen::Matrix<double, NUM_MEASUREMENT_DOF, NUM_STATE_DOF>;
using StateMatrix = Eigen::Matrix<double, NUM_STATE_DOF, NUM_STATE_DOF>;
using MeasureToStateMatrix = Eigen::Matrix<double, NUM_STATE_DOF, NUM_MEASUREMENT_DOF>;
using MeasureMatrix = Eigen::Matrix<double, NUM_MEASUREMENT_DOF, NUM_MEASUREMENT_DOF>;
using StateVector = Eigen::Matrix<double, NUM_STATE_DOF, 1>;
using PoseVector = Eigen::Matrix<double, NUM_MEASUREMENT_DOF, 1>;

struct KalmanFilter
{
    MeasureMatrix
        measurement_noise_cov;
    StateMatrix
        process_noise_cov,
        state_cov,
        state_cov_prior,
        transition_matrix;
    MeasureToStateMatrix
        kalman_gain;
    StateToMeasureMatrix
        measurement_matrix;
    StateVector
        state,
        state_prior;
    PoseVector
        innovation;
    void init();
    void time_update();
    void measurement_update(const PoseVector &measurement);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

struct KalmanProcessNoiseScaler
{
    MeasureMatrix
        innovation_cov_estimate;
    StateMatrix
        base_cov; // baseline (unscaled) process noise covariance matrix
    void init();
    void update(KalmanFilter &kf, double dt);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


struct DeadzoneFilter
{
    PoseVector
        last_output,
        dz_size;
    DeadzoneFilter() :
        last_output(PoseVector::Zero()),
        dz_size(PoseVector::Zero())
    {}
    void reset() {
        last_output = PoseVector::Zero();
    }
    PoseVector filter(const PoseVector &input);

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};


struct settings : opts {
    value<slider_value> noise_rot_slider_value;
    value<slider_value> noise_pos_slider_value;

    static constexpr double adaptivity_window_length = 0.25; // seconds
    static constexpr double deadzone_scale = 8;
    static constexpr double deadzone_exponent = 2.0;
    static constexpr double process_sigma_pos = 0.5;
    static constexpr double process_sigma_rot = 0.5;

    static double map_slider_value(const slider_value &v_)
    {
        const double v = v_;
#if 0
        //return std::pow(10., v * 4. - 3.);
#else
        constexpr int min_log10 = -3;
        constexpr int max_log10 = 1;
        constexpr int num_divisions = max_log10 - min_log10;
        /* ascii art representation of slider
          // ----- //  ------//  ------// ------- //   4 divisions
         -3    -   2         -1        0           1    power of 10
                   |      |
                   |      f + left_side_log10
                   |
                   left_side_log10
        */
        const int k = v * num_divisions; // in which division are we?!
        const double f = v * num_divisions - k; // where in the division are we?!
        const double ff = f * 9. + 1.;
        const double multiplier = int(ff * 10.) / 10.;
        const int left_side_log10 = min_log10 + k;
        const double val = std::pow(10., left_side_log10) * multiplier;
        return val;
#endif
    }

    settings() :
        opts("kalman-filter"),
        noise_rot_slider_value(b, "noise-rotation-slider", slider_value(0.5, 0., 1.)),
        noise_pos_slider_value(b, "noise-position-slider", slider_value(0.5, 0., 1.))
    {}

};

class kalman : public IFilter
{
    PoseVector do_kalman_filter(const PoseVector &input, double dt, bool new_input);
    void fill_transition_matrix(double dt);
    void fill_process_noise_cov_matrix(StateMatrix &target, double dt) const;
public:
    kalman();
    void reset();
    void filter(const double *input, double *output) override;
    void center() override { reset(); }
    module_status initialize() { return status_ok(); }
    PoseVector last_input;
    Timer timer;
    bool first_run;
    double dt_since_last_input;
    settings s;
    KalmanFilter kf;
    KalmanProcessNoiseScaler kf_adaptive_process_noise_cov;
    DeadzoneFilter dz_filter;
    slider_value prev_slider_pos[2];

    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

class kalmanDll : public Metadata
{
public:
    QString name() { return otr_tr("Kalman"); }
    QIcon icon() { return QIcon(":/images/filter-16.png"); }
};

class dialog_kalman: public IFilterDialog
{
    Q_OBJECT
public:
    dialog_kalman();
    Ui::KalmanUICdialog_kalman ui;
    void register_filter(IFilter*) override {}
    void unregister_filter() override {}
    settings s;
    kalman *filter;
public slots:
    void doOK();
    void doCancel();
    void updateLabels(const slider_value&);
};
