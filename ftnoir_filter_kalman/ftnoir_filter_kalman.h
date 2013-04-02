#pragma once
/* Copyright (c) 2013 Stanisław Halik <sthalik@misaki.pl>
 * 
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#ifndef INCLUDED_FTN_FILTER_H
#define INCLUDED_FTN_FILTER_H

#undef FTNOIR_TRACKER_BASE_LIB
#define FTNOIR_TRACKER_BASE_EXPORT Q_DECL_IMPORT

#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "ui_ftnoir_kalman_filtercontrols.h"
#include "facetracknoir/global-settings.h"
#include <opencv2/opencv.hpp>
#include <vector>
#include <QString>
#include <QIcon>
#include <QWidget>
#include <QObject>

#define DEFAULT_ACCL 750

class FTNOIR_FILTER_BASE_EXPORT FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    virtual ~FTNoIR_Filter() {
    }
    void Initialize();
    void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, THeadPoseData *last_post_filter, bool newTarget);
    cv::KalmanFilter kalman;
    double process_noise_covariance_matrix_all_values;
    double posteriori_error_covariance_matrix_all_values;
    double accl;
};

void kalman_load_settings(FTNoIR_Filter& self);
void kalman_save_settings(FTNoIR_Filter& self);

class FTNOIR_FILTER_BASE_EXPORT FTNoIR_FilterDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png"); }
};

class FTNOIR_FILTER_BASE_EXPORT FilterControls: public QWidget, Ui::KalmanUICFilterControls, public IFilterDialog
{
    Q_OBJECT
public:
    explicit FilterControls() : settingsDirty(false) {
        ui.setupUi(this);
        QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");    // Registry settings (in HK_USER)
        
        QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
        QSettings iniFile( currentFile, QSettings::IniFormat );     // Application settings (in INI-file)
        
        iniFile.beginGroup("ftnoir-filter-kalman");
        ui.post->setValue(iniFile.value("posteriori-error-covariance-matrix-all-values", "1e-12").toDouble());
        ui.pnoise->setValue(iniFile.value("process-noise-covariance-matrix-all-values", "1e-14").toDouble());
        ui.accl->setValue(iniFile.value("accel-coefficient", DEFAULT_ACCL).toDouble());
        iniFile.endGroup();
        connect(ui.btnOk, SIGNAL(clicked()), this, SLOT(doOK()));
        connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
        connect(ui.post, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        connect(ui.pnoise, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        connect(ui.accl, SIGNAL(valueChanged(double)), this, SLOT(settingsChanged(double)));
        show();
    }
    virtual ~FilterControls() {}
    void showEvent ( QShowEvent * event ) {
        show();
    }
    
    void Initialize(QWidget *parent, IFilter* ptr) {
    }
    
    bool settingsDirty;
    Ui::KalmanUICFilterControls ui;
    
public slots:
    void doOK();
    void doCancel();
    void settingsChanged(double unused) {
        settingsDirty = true;
    }
};

#endif
