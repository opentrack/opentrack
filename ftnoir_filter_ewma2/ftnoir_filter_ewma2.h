/********************************************************************************
* FaceTrackNoIR      This program is a private project of some enthusiastic     *
*                    gamers from Holland, who don't like to pay much for        *
*                    head-tracking.                                             *
*                                                                               *
* Copyright (C) 2012  Wim Vriend (Developing)                                   *
*                     Ron Hendriks (Researching and Testing)                    *
*                                                                               *
* Homepage                                                                      *
*                                                                               *
* This program is free software; you can redistribute it and/or modify it       *
* under the terms of the GNU General Public License as published by the         *
* Free Software Foundation; either version 3 of the License, or (at your        *
* option) any later version.                                                    *
*                                                                               *
* This program is distributed in the hope that it will be useful, but           *
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY    *
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for   *
* more details.                                                                 *
*                                                                               *
* You should have received a copy of the GNU General Public License along       *
* with this program; if not, see <http://www.gnu.org/licenses/>.                *
*                                                                               *
********************************************************************************/
#pragma once
#ifndef INCLUDED_FTN_FILTER_H
#define INCLUDED_FTN_FILTER_H

#include "ftnoir_filter_base/ftnoir_filter_base.h"
#include "facetracknoir/global-settings.h"
#include "ui_ftnoir_ewma_filtercontrols.h"
#include <QWidget>
#include <QMutex>
#include "facetracknoir/options.h"
using namespace options;

struct settings {
    pbundle b;
    value<int> kMinSmoothing, kMaxSmoothing, kSmoothingScaleCurve;
    settings() :
        b(bundle("ewma-filter")),
        kMinSmoothing(b, "min-smoothing", 15),
        kMaxSmoothing(b, "max-smoothing", 50),
        kSmoothingScaleCurve(b, "smoothing-scale-curve", 10)
    {}
};


class FTNoIR_Filter : public IFilter
{
public:
    FTNoIR_Filter();
    void reset() {}
    void FilterHeadPoseData(const double *target_camera_position,
                            double *new_camera_position);
    void receiveSettings();
private:
    bool first_run;
    double alpha_smoothing;
    double alpha[6];
    double current_camera_position[6];
    settings s;
};

//*******************************************************************************************************
// FaceTrackNoIR Filter Settings-dialog.
//*******************************************************************************************************

// Widget that has controls for FTNoIR protocol filter-settings.
class FilterControls: public QWidget, public IFilterDialog
{
    Q_OBJECT
public:
    FilterControls();
    void registerFilter(IFilter* flt);
    void unregisterFilter();

private:
    Ui::UICFilterControls ui;
    void save();
    settings s;
    FTNoIR_Filter* pFilter;

private slots:
    void doOK();
    void doCancel();
};

//*******************************************************************************************************
// FaceTrackNoIR Filter DLL. Functions used to get general info on the Filter
//*******************************************************************************************************
class FTNoIR_FilterDll : public Metadata
{
public:
    void getFullName(QString *strToBeFilled) { *strToBeFilled = QString("EWMA Filter Mk2"); }
    void getShortName(QString *strToBeFilled) { *strToBeFilled = QString("EWMA"); }
    void getDescription(QString *strToBeFilled) { *strToBeFilled = QString("Exponentially Weighted Moving Average filter with dynamic smoothing parameter"); }
    void getIcon(QIcon *icon){ *icon = QIcon(":/images/filter-16.png"); }
};

#endif  //INCLUDED_FTN_FILTER_H
//END
