#pragma once
/* Copyright (c) 2016 Michael Welter <mw.pub@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "compat/timer.hpp"
#include "api/plugin-api.hpp"
#include "options/options.hpp"
using namespace options;

// Eigen can't check for SSE3 on MSVC
#if defined _MSC_VER && defined __SSE2__
#   define EIGEN_VECTORIZE_SSE3
// this hardware is 10 years old
#   define EIGEN_VECTORIZE_SSE4_1
#   define EIGEN_VECTORIZE_SSE4_2
#endif

#include <Eigen/Core>
#include <Eigen/LU>

#include "ui_ftnoir_kalman_filtercontrols.h"
#include <QString>
#include <QWidget>

static constexpr inline int NUM_STATE_DOF = 12;
static constexpr inline int NUM_MEASUREMENT_DOF = 6;
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
};

struct KalmanProcessNoiseScaler
{
    MeasureMatrix
        innovation_cov_estimate;
    StateMatrix
        base_cov; // baseline (unscaled) process noise covariance matrix
    void init();
    void update(KalmanFilter &kf, double dt);
};


struct DeadzoneFilter
{
    PoseVector last_output { PoseVector::Zero() },
               dz_size     { PoseVector::Zero() };

    DeadzoneFilter() = default;
    void reset();
    PoseVector filter(const PoseVector &input);
};


struct settings : opts {
    value<slider_value> noise_rot_slider_value { b, "noise-rotation-slider", { .5, 0, 1 } };
    value<slider_value> noise_pos_slider_value { b, "noise-position-slider", { .5, 0, 1 } };

    static constexpr inline double adaptivity_window_length = 0.25; // seconds
    static constexpr inline double deadzone_scale = 8;
    static constexpr inline double deadzone_exponent = 2.0;
    static constexpr inline double process_sigma_pos = 0.5;
    static constexpr inline double process_sigma_rot = 0.5;

    static double map_slider_value(const slider_value &v);

    settings() : opts("kalman-filter") {}
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
    module_status initialize() override { return status_ok(); }

    double dt_since_last_input;
    PoseVector last_input;
    KalmanFilter kf;
    KalmanProcessNoiseScaler kf_adaptive_process_noise_cov;
    DeadzoneFilter dz_filter;
    settings s;
    slider_value prev_slider_pos[2] {
        *s.noise_pos_slider_value,
        *s.noise_rot_slider_value,
    };
    Timer timer;

    bool first_run = true;
};

class kalmanDll : public Metadata
{
    Q_OBJECT

    QString name() override { return tr("Kalman"); }
    QIcon icon() override { return QIcon(":/images/filter-16.png"); }
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
