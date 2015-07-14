/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_kalman.h"
#include "opentrack/plugin-api.hpp"
#include <QDebug>
#include <cmath>

FTNoIR_Filter::FTNoIR_Filter() {
    reset();
    prev_slider_pos = s.noise_stddev_slider;
}

// the following was written by Donovan Baarda <abo@minkirri.apana.org.au>
// https://sourceforge.net/p/facetracknoir/discussion/1150909/thread/418615e1/?limit=25#af75/084b
void FTNoIR_Filter::reset() {
    // Setup kalman with state (x) is the 6 tracker outputs then
    // their 6 corresponding velocities, and the measurement (z) is
    // the 6 tracker outputs.
    kalman.init(12, 6, 0, CV_64F);
    // Initialize the transitionMatrix and processNoiseCov for
    // dt=0.1. This needs to be updated each frame for the real dt
    // value, but this hows you what they should look like. See
    // http://en.wikipedia.org/wiki/Kalman_filter#Example_application.2C_technical
    double dt = 0.1;
    kalman.transitionMatrix = (cv::Mat_<double>(12, 12) <<
    1, 0, 0, 0, 0, 0, dt, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, dt, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, dt, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, dt, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, dt, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, dt,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    double accel_variance = accel_stddev * accel_stddev;
    double a = dt * dt * accel_variance;  // dt^2 * accel_variance.
    double b = 0.5 * a * dt;  // (dt^3)/2 * accel_variance.
    double c = 0.5 * b * dt;  // (dt^4)/4 * accel_variance.
    kalman.processNoiseCov = (cv::Mat_<double>(12, 12) <<
    c, 0, 0, 0, 0, 0, b, 0, 0, 0, 0, 0,
    0, c, 0, 0, 0, 0, 0, b, 0, 0, 0, 0,
    0, 0, c, 0, 0, 0, 0, 0, b, 0, 0, 0,
    0, 0, 0, c, 0, 0, 0, 0, 0, b, 0, 0,
    0, 0, 0, 0, c, 0, 0, 0, 0, 0, b, 0,
    0, 0, 0, 0, 0, c, 0, 0, 0, 0, 0, b,
    b, 0, 0, 0, 0, 0, a, 0, 0, 0, 0, 0,
    0, b, 0, 0, 0, 0, 0, a, 0, 0, 0, 0,
    0, 0, b, 0, 0, 0, 0, 0, a, 0, 0, 0,
    0, 0, 0, b, 0, 0, 0, 0, 0, a, 0, 0,
    0, 0, 0, 0, b, 0, 0, 0, 0, 0, a, 0,
    0, 0, 0, 0, 0, b, 0, 0, 0, 0, 0, a);
    cv::setIdentity(kalman.measurementMatrix);
    const double noise_stddev = (1+s.noise_stddev_slider) * s.mult_noise_stddev;
    const double noise_variance = noise_stddev * noise_stddev;
    cv::setIdentity(kalman.measurementNoiseCov, cv::Scalar::all(noise_variance));
    cv::setIdentity(kalman.errorCovPost, cv::Scalar::all(accel_variance * 1e4));
    for (int i = 0; i < 6; i++) {
        last_input[i] = 0;
    }
    timer.invalidate();
}

void FTNoIR_Filter::filter(const double* input, double *output)
{
    if (prev_slider_pos != s.noise_stddev_slider)
    {
        reset();
        prev_slider_pos = s.noise_stddev_slider;
    }
    // Start the timer if it's not running.
    if (!timer.isValid())
        timer.start();
    // Get the time in seconds since last run and restart the timer.
    auto dt = timer.restart() / 1000.0f;
    // Note this is a terrible way to detect when there is a new
    // frame of tracker input, but it is the best we have.
    bool new_input = false;
    for (int i = 0; i < 6 && !new_input; i++)
        new_input = (input[i] != last_input[i]);
    // Update the transitionMatrix and processNoiseCov for dt.
    double accel_variance = accel_stddev * accel_stddev;
    double a = dt * dt * accel_variance;  // dt^2 * accel_variance.
    double b = 0.5 * a * dt;  // (dt^3)/2 * accel_variance.
    double c = 0.5 * b * dt;  // (dt^4)/4 * accel_variance.
    for (int i = 0; i < 6; i++) {
        kalman.transitionMatrix.at<double>(i,i+6) = dt;
        kalman.processNoiseCov.at<double>(i,i) = c;
        kalman.processNoiseCov.at<double>(i+6,i+6) = a;
        kalman.processNoiseCov.at<double>(i,i+6) = b;
        kalman.processNoiseCov.at<double>(i+6,i) = b;
    }
    // Get the updated predicted position.
    cv::Mat next_output = kalman.predict();
    // If we have new tracker input, get the corrected position.
    if (new_input) {
        cv::Mat measurement(6, 1, CV_64F);
        for (int i = 0; i < 6; i++) {
            measurement.at<double>(i) = input[i];
            // Save last_input for detecting new tracker input.
            last_input[i] = input[i];
        }
        next_output = kalman.correct(measurement);
    }
    // Set output to the next_output.
    for (int i = 0; i < 6; i++) {
        output[i] = next_output.at<double>(i);
    }
}

void FilterControls::doOK() {
    s.b->save();
    close();
}

void FilterControls::doCancel() {
    s.b->reload();
    close();
}

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
    return new FTNoIR_FilterDll;
}

extern "C" OPENTRACK_EXPORT IFilter* GetConstructor()
{
    return new FTNoIR_Filter;
}

extern "C" OPENTRACK_EXPORT IFilterDialog* GetDialog() {
    return new FilterControls;
}
