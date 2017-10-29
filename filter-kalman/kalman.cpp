/* Copyright (c) 2016 Michael Welter <mw.pub@welter-4d.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "kalman.h"
#include <QDebug>
#include <cmath>

constexpr double settings::adaptivity_window_length;
constexpr double settings::deadzone_scale;
constexpr double settings::deadzone_exponent;
constexpr double settings::process_sigma_pos;
constexpr double settings::process_sigma_rot;

void KalmanFilter::init()
{
    // allocate and initialize matrices
    measurement_noise_cov = MeasureMatrix::Zero();
    process_noise_cov = StateMatrix::Zero();
    state_cov = StateMatrix::Zero();
    state_cov_prior = StateMatrix::Zero();
    transition_matrix = StateMatrix::Zero();
    measurement_matrix = StateToMeasureMatrix::Zero();
    kalman_gain = MeasureToStateMatrix::Zero();
    // initialize state variables
    state = StateVector::Zero();
    state_prior = StateVector::Zero();
    innovation = PoseVector::Zero();
}


void KalmanFilter::time_update()
{
    state_prior     = transition_matrix * state;
    state_cov_prior = transition_matrix * state_cov * transition_matrix.transpose() + process_noise_cov;
}


void KalmanFilter::measurement_update(const PoseVector &measurement)
{
    MeasureMatrix tmp     = measurement_matrix * state_cov_prior * measurement_matrix.transpose() + measurement_noise_cov;
    MeasureMatrix tmp_inv = tmp.inverse();
    kalman_gain = state_cov_prior * measurement_matrix.transpose() * tmp_inv;
    innovation = measurement - measurement_matrix * state_prior;
    state     = state_prior + kalman_gain * innovation;
    state_cov = state_cov_prior - kalman_gain * measurement_matrix * state_cov_prior;
}



void KalmanProcessNoiseScaler::init()
{
    base_cov = StateMatrix::Zero(NUM_STATE_DOF, NUM_STATE_DOF);
    innovation_cov_estimate = MeasureMatrix::Zero(NUM_MEASUREMENT_DOF, NUM_MEASUREMENT_DOF);
}


/* Uses
    innovation, measurement_matrix, measurement_noise_cov, and state_cov_prior
   found in KalmanFilter. It sets
    process_noise_cov
*/
void KalmanProcessNoiseScaler::update(KalmanFilter &kf, double dt)
{
    MeasureMatrix ddT = kf.innovation * kf.innovation.transpose();
    double f = dt / (dt + settings::adaptivity_window_length);
    innovation_cov_estimate =
        f * ddT + (1. - f) * innovation_cov_estimate;

    double T1 = (innovation_cov_estimate - kf.measurement_noise_cov).trace();
    double T2 = (kf.measurement_matrix * kf.state_cov_prior * kf.measurement_matrix.transpose()).trace();
    double alpha = 0.001;
    if (T2 > 0. && T1 > 0.)
    {
        alpha = T1 / T2;
        alpha = std::sqrt(alpha);
        alpha = std::fmin(1000., std::fmax(0.001, alpha));
    }
    kf.process_noise_cov = alpha * base_cov;
    //qDebug() << "alpha = " << alpha;
}


PoseVector DeadzoneFilter::filter(const PoseVector &input)
{
    PoseVector out;
    for (int i = 0; i < input.rows(); ++i)
    {
        const double dz = dz_size[i];
        if (dz > 0.)
        {
            const double delta = input[i] - last_output[i];
            const double f = std::pow(std::fabs(delta) / dz, settings::deadzone_exponent);
            const double response = f / (f + 1.) * delta;
            out[i] = last_output[i] + response;
        }
        else
            out[i] = input[i];
        last_output[i] = out[i];
    }
    return out;
}


void kalman::fill_transition_matrix(double dt)
{
    for (int i = 0; i < 6; ++i)
    {
        kf.transition_matrix(i, i + 6) = dt;
    }
}

void kalman::fill_process_noise_cov_matrix(StateMatrix &target, double dt) const
{
    // This model is like movement at fixed velocity plus superimposed
    // brownian motion. Unlike standard models for tracking of objects
    // with a very well predictable trajectory (e.g.
    // https://en.wikipedia.org/wiki/Kalman_filter#Example_application.2C_technical)
    double sigma_pos = s.process_sigma_pos;
    double sigma_angle = s.process_sigma_rot;
    double a_pos = sigma_pos * sigma_pos * dt;
    double a_ang = sigma_angle * sigma_angle * dt;
    constexpr double b = 20;
    constexpr double c = 1.;
    for (int i = 0; i < 3; ++i)
    {
        target(i, i) = a_pos;
        target(i, i + 6) = a_pos * c;
        target(i + 6, i) = a_pos * c;
        target(i + 6, i + 6) = a_pos * b;
    }
    for (int i = 3; i < 6; ++i)
    {
        target(i, i) = a_ang;
        target(i, i + 6) = a_ang * c;
        target(i + 6, i) = a_ang * c;
        target(i + 6, i + 6) = a_ang * b;
    }
}


PoseVector kalman::do_kalman_filter(const PoseVector &input, double dt, bool new_input)
{
    if (new_input)
    {
        dt = dt_since_last_input;
        fill_transition_matrix(dt);
        fill_process_noise_cov_matrix(kf_adaptive_process_noise_cov.base_cov, dt);
        kf_adaptive_process_noise_cov.update(kf, dt);
        kf.time_update();
        kf.measurement_update(input);
    }
    return kf.state.head(6);
}



kalman::kalman() {
    reset();
}

// The original code was written by Donovan Baarda <abo@minkirri.apana.org.au>
// https://sourceforge.net/p/facetracknoir/discussion/1150909/thread/418615e1/?limit=25#af75/084b
void kalman::reset()
{
    kf.init();
    kf_adaptive_process_noise_cov.init();
    for (int i = 0; i < 6; ++i)
    {
        // initialize part of the transition matrix that do not change.
        kf.transition_matrix(i, i) = 1.;
        kf.transition_matrix(i + 6, i + 6) = 1.;
        // "extract" positions, i.e. the first 6 state dof.
        kf.measurement_matrix(i, i) = 1.;
    }

    double noise_variance_position = settings::map_slider_value(s.noise_pos_slider_value);
    double noise_variance_angle = settings::map_slider_value(s.noise_rot_slider_value);
    for (int i = 0; i < 3; ++i)
    {
        kf.measurement_noise_cov(i    , i    ) = noise_variance_position;
        kf.measurement_noise_cov(i + 3, i + 3) = noise_variance_angle;
    }

    fill_transition_matrix(0.03);
    fill_process_noise_cov_matrix(kf_adaptive_process_noise_cov.base_cov, 0.03);

    kf.process_noise_cov = kf_adaptive_process_noise_cov.base_cov;
    kf.state_cov = kf.process_noise_cov;

    for (int i = 0; i < 6; i++) {
        last_input[i] = 0;
    }
    first_run = true;
    dt_since_last_input = 0;

    prev_slider_pos[0] = static_cast<slider_value>(s.noise_pos_slider_value);
    prev_slider_pos[1] = static_cast<slider_value>(s.noise_rot_slider_value);

    dz_filter.reset();
}


void kalman::filter(const double* input_, double *output_)
{
    // almost non-existent cost, so might as well ...
    Eigen::Map<const PoseVector> input(input_, PoseVector::RowsAtCompileTime, 1);
    Eigen::Map<PoseVector> output(output_, PoseVector::RowsAtCompileTime, 1);

    if (!(prev_slider_pos[0] == s.noise_pos_slider_value &&
          prev_slider_pos[1] == s.noise_rot_slider_value))
    {
        reset();
    }

    // Start the timer on first filter evaluation.
    if (first_run)
    {
        timer.start();
        first_run = false;
        return;
    }

    // Note this is a terrible way to detect when there is a new
    // frame of tracker input, but it is the best we have.
    bool new_input = input.cwiseNotEqual(last_input).any();

    // Get the time in seconds since last run and restart the timer.
    const double dt = timer.elapsed_seconds();
    dt_since_last_input += dt;
    timer.start();

    output = do_kalman_filter(input, dt, new_input);

    {
        // Compute deadzone size base on estimated state variance.
        // Given a constant input plus measurement noise, KF should converge to the true input.
        // This works well. That is the output pose becomes very still afte some time.
        // The QScaling adaptive filter makes the state cov vary depending on the estimated noise
        // and the measured noise of the innovation sequence. After a sudden movement it peaks
        // and then decays asymptotically to some constant value taken in stationary state. 
        // We can use this to calculate the size of the deadzone, so that in the stationary state the
        // deadzone size is small. Thus the tracking error due to the dz-filter becomes also small.
        PoseVector variance = kf.state_cov.diagonal().head(6);
        dz_filter.dz_size = variance.cwiseSqrt() * s.deadzone_scale;
    }
    output = dz_filter.filter(output);

    if (new_input)
    {
        dt_since_last_input = 0;
        last_input = input;
    }
}



dialog_kalman::dialog_kalman()
    : filter(nullptr)
{
    ui.setupUi(this);
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.noise_rot_slider_value, ui.noiseRotSlider);
    tie_setting(s.noise_pos_slider_value, ui.noisePosSlider);

    connect(&s.noise_rot_slider_value, SIGNAL(valueChanged(const slider_value&)), this, SLOT(updateLabels(const slider_value&)));
    connect(&s.noise_pos_slider_value, SIGNAL(valueChanged(const slider_value&)), this, SLOT(updateLabels(const slider_value&)));

    updateLabels(slider_value());
}


void dialog_kalman::updateLabels(const slider_value&)
{
    // M$ hates unicode! (M$ autoconverts source code of one kind of utf-8 format, 
    // the one without BOM, to another kind that QT does not like)
    // Previous attempt to use c++11 utf8 strings like u8" °" now failed for unkown 
    // reasons where it worked before. Hence fallback to QChar(0x00b0).
    this->ui.noiseRotLabel->setText(
        QString::number(settings::map_slider_value(s.noise_rot_slider_value), 'f', 3) + " " + QChar(0x00b0));

    this->ui.noisePosLabel->setText(
        QString::number(settings::map_slider_value(s.noise_pos_slider_value), 'f', 3) + " cm");
}


void dialog_kalman::doOK() {
    s.b->save();
    close();
}


void dialog_kalman::doCancel()
{
    close();
}


OPENTRACK_DECLARE_FILTER(kalman, dialog_kalman, kalmanDll)
