/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_kalman.h"
#include "facetracknoir/global-settings.h"
#include <QDebug>
#include <math.h>

void kalman_load_settings(FTNoIR_Filter&) {
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
    
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
    
    iniFile.beginGroup("ftnoir-filter-kalman");
    iniFile.endGroup();
}

void kalman_save_settings(FilterControls&) {
    QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
    
    QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
    QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
    
    iniFile.beginGroup("ftnoir-filter-kalman");
    iniFile.endGroup();
}

FTNoIR_Filter::FTNoIR_Filter() {
    kalman_load_settings(*this);
    Initialize();
}

// the following was written by Donovan Baarda <abo@minkirri.apana.org.au>
// https://sourceforge.net/p/facetracknoir/discussion/1150909/thread/418615e1/?limit=25#af75/084b
void FTNoIR_Filter::Initialize() {
    const double accel_variance = 1e-3;
    const double noise_variance = 5e2;
    kalman.init(12, 6, 0, CV_64F);
    kalman.transitionMatrix = (cv::Mat_<double>(12, 12) <<
    1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1,
    0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1);
    double a = 0.25 * accel_variance;
    double b = 0.5 * accel_variance;
    double c = 1.0 * accel_variance;
    kalman.processNoiseCov = (cv::Mat_<double>(12, 12) <<
    a, 0, 0, 0, 0, 0, b, 0, 0, 0, 0, 0,
    0, a, 0, 0, 0, 0, 0, b, 0, 0, 0, 0,
    0, 0, a, 0, 0, 0, 0, 0, b, 0, 0, 0,
    0, 0, 0, a, 0, 0, 0, 0, 0, b, 0, 0,
    0, 0, 0, 0, a, 0, 0, 0, 0, 0, b, 0,
    0, 0, 0, 0, 0, a, 0, 0, 0, 0, 0, b,
    b, 0, 0, 0, 0, 0, c, 0, 0, 0, 0, 0,
    0, b, 0, 0, 0, 0, 0, c, 0, 0, 0, 0,
    0, 0, b, 0, 0, 0, 0, 0, c, 0, 0, 0,
    0, 0, 0, b, 0, 0, 0, 0, 0, c, 0, 0,
    0, 0, 0, 0, b, 0, 0, 0, 0, 0, c, 0,
    0, 0, 0, 0, 0, b, 0, 0, 0, 0, 0, c);
    cv::setIdentity(kalman.measurementMatrix);
    cv::setIdentity(kalman.measurementNoiseCov, cv::Scalar::all(noise_variance));
    cv::setIdentity(kalman.errorCovPost, cv::Scalar::all(accel_variance * 1e4));
    for (int i = 0; i < 6; i++)
    {
        prev_position[i] = 0;
        prev2_filter_pos[i] = 0;
        prev_filter_pos[i] = 0;
        timedelta = 1;
        timer.invalidate();
    }
}

template<typename T>
static inline T clamp(const T min, const T max, const T value)
{
    if (value < min)
        return min;
    if (value > max)
        return max;
    return value;
}

void FTNoIR_Filter::FilterHeadPoseData(const double* target_camera_position,
                                       double *new_camera_position)
{
    bool new_target = false;
    
    for (int i = 0; i < 6; i++)
        if (prev_position[i] != target_camera_position[i])
        {
            new_target = true;
            break;
        }

    if (new_target) {
        cv::Mat output = kalman.predict();
        cv::Mat measurement(6, 1, CV_64F);
        for (int i = 0; i < 3; i++) {
            measurement.at<double>(i) = target_camera_position[i+3];
            measurement.at<double>(i+3) = target_camera_position[i];
        }
        kalman.correct(measurement);
        for (int i = 0; i < 6; i++)
        {
            prev_position[i] = target_camera_position[i];
        }
        if (timer.isValid())
            timedelta = timer.elapsed();
        else
            timedelta = 1;
        for (int i = 0; i < 6; i++)
        {
            prev2_filter_pos[i] = prev_filter_pos[i];
            prev_filter_pos[i] = new_camera_position[i] = output.at<double>((i + 3) % 6);
        }
        timer.start();
    } else {
        auto d = timer.isValid() ? timer.elapsed() : 1;
        auto c = clamp(0.0, 1.0, d / (double) timedelta);
        for (int i = 0; i < 6; i++)
            new_camera_position[i] = prev2_filter_pos[i] + (prev_filter_pos[i] - prev2_filter_pos[i]) * c;
    }
}

void FilterControls::doOK() {
    kalman_save_settings(*this);
    close();
}

void FilterControls::doCancel() {
    if (settingsDirty) {
        int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );
        
        qDebug() << "doCancel says: answer =" << ret;
        
        switch (ret) {
            case QMessageBox::Save:
                kalman_save_settings(*this);
                this->close();
                break;
            case QMessageBox::Discard:
                this->close();
                break;
            case QMessageBox::Cancel:
                // Cancel was clicked
                break;
            default:
                // should never be reached
                break;
        }
    }
    else {
        this->close();
    }
}

extern "C" FTNOIR_FILTER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
    return new FTNoIR_FilterDll;
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilterDialog* CALLING_CONVENTION GetDialog() {
    return new FilterControls;
}
