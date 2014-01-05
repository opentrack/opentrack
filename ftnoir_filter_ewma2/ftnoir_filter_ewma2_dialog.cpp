/********************************************************************************
* FaceTrackNoIR		This program is a private project of some enthusiastic		*
*					gamers from Holland, who don't like to pay much for			*
*					head-tracking.												*
*																				*
* Copyright (C) 2012	Wim Vriend (Developing)									*
*						Ron Hendriks (Researching and Testing)					*
*																				*
* Homepage																		*
*																				*
* This program is free software; you can redistribute it and/or modify it		*
* under the terms of the GNU General Public License as published by the			*
* Free Software Foundation; either version 3 of the License, or (at your		*
* option) any later version.													*
*																				*
* This program is distributed in the hope that it will be useful, but			*
* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY	*
* or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for	*
* more details.																	*
*																				*
* You should have received a copy of the GNU General Public License along		*
* with this program; if not, see <http://www.gnu.org/licenses/>.				*
*																				*
********************************************************************************/
#include "ftnoir_filter_ewma2.h"
#include "math.h"
#include <QDebug>
#include "facetracknoir/global-settings.h"
#include "ui_ftnoir_ewma_filtercontrols.h"

FilterControls::FilterControls() :
    QWidget(), pFilter(NULL)
{
    ui.setupUi( this );

    connect(ui.btnOK, SIGNAL(clicked()), this, SLOT(doOK()));
    connect(ui.btnCancel, SIGNAL(clicked()), this, SLOT(doCancel()));

    tie_setting(s.kMaxSmoothing, ui.maxSmooth);
    tie_setting(s.kMinSmoothing, ui.minSmooth);
    tie_setting(s.kSmoothingScaleCurve, ui.powCurve);
}

void FilterControls::registerFilter(IFilter* flt)
{
    pFilter = (FTNoIR_Filter*) flt;
}

void FilterControls::unregisterFilter()
{
    pFilter = NULL;
}

void FilterControls::doOK() {
	save();
	this->close();
}

void FilterControls::doCancel() {
    s.b->revert();
    this->close();
}

void FilterControls::save() {
    s.b->save();
    if (pFilter)
        pFilter->receiveSettings();
}

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilterDialog* CALLING_CONVENTION GetDialog( )
{
    return new FilterControls;
}
