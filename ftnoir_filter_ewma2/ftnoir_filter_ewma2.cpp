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
#include <QWidget>
#include "facetracknoir/global-settings.h"
#include <algorithm>
//#define LOG_OUTPUT

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FTNoIR_Filter::FTNoIR_Filter()
{
	first_run = true;
	alpha_smoothing = 0.02f;		// this is a constant for now, might be a parameter later
	loadSettings();					// Load the Settings

}

FTNoIR_Filter::~FTNoIR_Filter()
{

}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Filter::loadSettings() {
	qDebug() << "FTNoIR_Filter::loadSettings says: Starting ";
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter::loadSettings says: iniFile = " << currentFile;

	//
	// The EWMA2-filter-settings are in the Tracking group: this is because they used to be on the Main Form of FaceTrackNoIR
	//
	iniFile.beginGroup ( "Tracking" );
	kMinSmoothing = iniFile.value ( "minSmooth", 15 ).toInt();
	kMaxSmoothing = iniFile.value ( "maxSmooth", 50 ).toInt();
	kSmoothingScaleCurve = iniFile.value ( "powCurve", 10 ).toInt();
	iniFile.endGroup ();

}

void FTNoIR_Filter::FilterHeadPoseData(double *current_camera_position, double *target_camera_position, double *new_camera_position, double *last_post_filter)
{
	//non-optimised version for clarity
    double prev_output[6];
    double target[6];
    double output_delta[6];
    double scale[]={0.025f,0.025f,0.025f,6.0f,6.0f,6.0f};
    double norm_output_delta[6];
    double output[6];

    for (int i = 0; i < 6; i++)
    {
        prev_output[i] = current_camera_position[i];
        target[i] = target_camera_position[i];
    }

	if (first_run==true)
	{
		//on the first run, output=target
        for (int i=0;i<6;i++)
		{
			output[i]=target[i];
			prev_alpha[i] = 0.0f;
		}

        for (int i = 0; i < 6; i++)
            new_camera_position[i] = target[i];

		first_run=false;
		
		//we can bail
		return;
	}

	//how far does the camera need to move to catch up?
    for (int i=0;i<6;i++)
	{
		output_delta[i]=(target[i]-prev_output[i]);
	}

	//normalise the deltas
    for (int i=0;i<6;i++)
	{
        norm_output_delta[i]=std::min<double>(std::max<double>(fabs(output_delta[i])/scale[i],0.0),1.0);
	}

	//calculate the alphas
	//work out the dynamic smoothing factors
//	if (newTarget) {
        for (int i=0;i<6;i++)
		{
            alpha[i]=1.0/(kMinSmoothing+((1.0-pow(norm_output_delta[i],kSmoothingScaleCurve))*smoothing_frames_range));
			smoothed_alpha[i]=(alpha_smoothing*alpha[i])+((1.0f-alpha_smoothing)*prev_alpha[i]);
		}
//	}

	//qDebug() << "FTNoIR_Filter::FilterHeadPoseData() smoothing frames = " << smoothing_frames_range;
	//qDebug() << "FTNoIR_Filter::FilterHeadPoseData() alpha[3] = " << alpha[3];

	//use the same (largest) smoothed alpha for each channel
	//NB: larger alpha = *less* lag (opposite to what you'd expect)
	float largest_alpha=0.0f;
    for (int i=0;i<6;i++)
	{
		if (smoothed_alpha[i]>=largest_alpha)
		{
			largest_alpha=smoothed_alpha[i];
		}
	}

	//move the camera
    for (int i=0;i<6;i++)
	{
		output[i]=(largest_alpha*target[i])+((1.0f-largest_alpha)*prev_output[i]);
//		output[i]=(smoothed_alpha[i]*target[i])+((1.0f-smoothed_alpha[i])*prev_output[i]);
	}


	#ifdef LOG_OUTPUT
	//	Use this for some debug-output to file...
	QFile data(QCoreApplication::applicationDirPath() + "\\EWMA_output.txt");
	if (data.open(QFile::WriteOnly | QFile::Append)) {
		QTextStream out(&data);
		out << "output:\t" << output[0] << "\t" << output[1] << "\t" << output[2] << "\t" << output[3] << "\t" << output[4] << "\t" << output[5] << '\n';
		out << "target:\t" << target[0] << "\t" << target[1] << "\t" << target[2] << "\t" << target[3] << "\t" << target[4] << "\t" << target[5] << '\n';
		out << "prev_output:\t" << prev_output[0] << "\t" << prev_output[1] << "\t" << prev_output[2] << "\t" << prev_output[3] << "\t" << prev_output[4] << "\t" << prev_output[5] << '\n';
		out << "largest_alpha:\t" << largest_alpha << '\n';
	}
	#endif

    for (int i = 0; i < 6; i++)
    {
        new_camera_position[i] = output[i];
        current_camera_position[i] = output[i];
    }

	//update filter memories ready for next sample
    for (int i=0;i<6;i++)
	{
		prev_alpha[i]=smoothed_alpha[i];
	}
	return;
}

////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter object.

// Export both decorated and undecorated names.
//   GetFilter     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetFilter@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetFilter=_GetFilter@0")

extern "C" FTNOIR_FILTER_BASE_EXPORT IFilter* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Filter;
}
