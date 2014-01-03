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
#include <QElapsedTimer>
#include <QObject>
#include "facetracknoir/options.h"
using namespace options;

class FTNOIR_FILTER_BASE_EXPORT FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void reset() virt_override;
    void FilterHeadPoseData(const double *target_camera_position,
                            double *new_camera_position) virt_override;
    cv::KalmanFilter kalman;
    double prev_position[6];
    double prev2_filter_pos[6];
    double prev_filter_pos[6];
    QElapsedTimer timer;
    qint64 timedelta;
};

class FTNOIR_FILTER_BASE_EXPORT FTNoIR_FilterDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Kalman filter"); }
    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png"); }
};

class FTNOIR_FILTER_BASE_EXPORT FilterControls: public QWidget, public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls() {
        ui.setupUi(this);
        connect(ui.btnOk, SIGNAL(clicked()), this, SLOT(doOK()));
        connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
        show();
    }
    Ui::KalmanUICFilterControls ui;
    virtual void registerFilter(IFilter*) virt_override {}
    virtual void unregisterFilter() virt_override {}
public slots:
    void doOK();
    void doCancel();
};

#endif
