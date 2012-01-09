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
#include "ftnoir_filter_EWMA2.h"
#include "math.h"
#include <QDebug>

//#define LOG_OUTPUT

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

FTNoIR_Filter_EWMA2::FTNoIR_Filter_EWMA2()
{
	//populate the description strings
	filterFullName = "EWMA Filter Mk2";
	filterShortName = "EWMA";
	filterDescription = "Exponentially Weighted Moving Average filter with dynamic smoothing parameter";

	//allocate memory for the parameters
	parameterValueAsFloat.clear();
	parameterRange.clear();
	parameterSteps.clear();
	parameterNameAsString.clear();
	parameterValueAsString.clear();
	parameterUnitsAsString.clear();

	//set up parameters
	parameterNameAsString.append("MinSmoothing");
	parameterUnitsAsString.append("Frames");
	parameterRange.append(std::pair<float,float>(1.0f,100.0f));
	parameterSteps.append(1.0f);
	parameterValueAsFloat.append(0.0f);
	parameterValueAsString.append("");
	setParameterValue(kMinSmoothing,10.0f);

	parameterNameAsString.append("MaxSmoothing");
	parameterUnitsAsString.append("Frames");
	parameterRange.append(std::pair<float,float>(1.0f,100.0f));
	parameterSteps.append(1.0f);
	parameterValueAsFloat.append(0.0f);
	parameterValueAsString.append("");
	setParameterValue(kMaxSmoothing,50.0f);

	parameterNameAsString.append("SmoothingScaleCurve");
	parameterUnitsAsString.append("Power");
	parameterRange.append(std::pair<float,float>(0.25f,10.0f));
	parameterSteps.append(0.0f);
	parameterValueAsFloat.append(0.0f);
	parameterValueAsString.append("");
	setParameterValue(kSmoothingScaleCurve,10.0f);

	first_run = true;
	alpha_smoothing = 0.02f;		// this is a constant for now, might be a parameter later
	loadSettings();					// Load the Settings

}

FTNoIR_Filter_EWMA2::~FTNoIR_Filter_EWMA2()
{

}

void FTNoIR_Filter_EWMA2::Release()
{
    delete this;
}

void FTNoIR_Filter_EWMA2::Initialize()
{
	qDebug() << "FTNoIR_Filter_EWMA2::Initialize says: Starting ";
	loadSettings();
	return;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Filter_EWMA2::loadSettings() {
	qDebug() << "FTNoIR_Filter_EWMA2::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Filter_EWMA2::loadSettings says: iniFile = " << currentFile;

	//
	// The EWMA2-filter-settings are in the Tracking group: this is because they used to be on the Main Form of FaceTrackNoIR
	//
	iniFile.beginGroup ( "Tracking" );
	setParameterValue(0, iniFile.value ( "minSmooth", 15 ).toInt());
	setParameterValue(1, iniFile.value ( "maxSmooth", 50 ).toInt());
	setParameterValue(2, iniFile.value ( "powCurve", 10 ).toInt());
	iniFile.endGroup ();

}

void FTNoIR_Filter_EWMA2::FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget)
{
	//non-optimised version for clarity
	float prev_output[6];
	float target[6];
	float output_delta[6];
	float scale[]={0.025f,0.025f,0.025f,6.0f,6.0f,6.0f};
	float norm_output_delta[6];
	float output[6];
	int i=0;

	#if PRE_FILTER_SCALING
	//compensate for any prefilter scaling
	scale[0]*=X_POS_SCALE;
	scale[1]*=Y_POS_SCALE;
	scale[2]*=Z_POS_SCALE;
	scale[3]*=X_ROT_SCALE;
	scale[4]*=Y_ROT_SCALE;
	scale[5]*=Z_ROT_SCALE;
	#endif

	//find out how far the head has moved
	prev_output[0]=current_camera_position->x;
	prev_output[1]=current_camera_position->y;
	prev_output[2]=current_camera_position->z;
	prev_output[3]=current_camera_position->yaw;
	prev_output[4]=current_camera_position->pitch;
	prev_output[5]=current_camera_position->roll;

	target[0]=target_camera_position->x;
	target[1]=target_camera_position->y;
	target[2]=target_camera_position->z;
	target[3]=target_camera_position->yaw;
	target[4]=target_camera_position->pitch;
	target[5]=target_camera_position->roll;

	if (first_run==true)
	{
		//on the first run, output=target
		for (i=0;i<6;i++)
		{
			output[i]=target[i];
			prev_alpha[i] = 0.0f;
		}

		new_camera_position->x=target[0];
		new_camera_position->y=target[1];
		new_camera_position->z=target[2];
		new_camera_position->yaw=target[3];
		new_camera_position->pitch=target[4];
		new_camera_position->roll=target[5];

		first_run=false;
		
		//we can bail
		return;
	}

	//how far does the camera need to move to catch up?
	for (i=0;i<6;i++)
	{
		output_delta[i]=(target[i]-prev_output[i]);
	}

	//normalise the deltas
	for (i=0;i<6;i++)
	{
		norm_output_delta[i]=std::min(std::max(fabs(output_delta[i])/scale[i],0.0f),1.0f);
	}

	//calculate the alphas
	//work out the dynamic smoothing factors
//	if (newTarget) {
		for (i=0;i<6;i++)
		{
			alpha[i]=1.0f/(parameterValueAsFloat[kMinSmoothing]+((1.0f-pow(norm_output_delta[i],parameterValueAsFloat[kSmoothingScaleCurve]))*smoothing_frames_range));
			smoothed_alpha[i]=(alpha_smoothing*alpha[i])+((1.0f-alpha_smoothing)*prev_alpha[i]);
		}
//	}

	//qDebug() << "FTNoIR_Filter_EWMA2::FilterHeadPoseData() smoothing frames = " << smoothing_frames_range;
	//qDebug() << "FTNoIR_Filter_EWMA2::FilterHeadPoseData() alpha[3] = " << alpha[3];

	//use the same (largest) smoothed alpha for each channel
	//NB: larger alpha = *less* lag (opposite to what you'd expect)
	float largest_alpha=0.0f;
	for (i=0;i<6;i++)
	{
		if (smoothed_alpha[i]>=largest_alpha)
		{
			largest_alpha=smoothed_alpha[i];
		}
	}

	//move the camera
	for (i=0;i<6;i++)
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

	new_camera_position->x=output[0];
	new_camera_position->y=output[1];
	new_camera_position->z=output[2];
	new_camera_position->yaw=output[3];
	new_camera_position->pitch=output[4];
	new_camera_position->roll=output[5];

	//
	// Also update the 'current' position, for the next iteration.
	//
	current_camera_position->x=output[0];
	current_camera_position->y=output[1];
	current_camera_position->z=output[2];
	current_camera_position->yaw=output[3];
	current_camera_position->pitch=output[4];
	current_camera_position->roll=output[5];

	//update filter memories ready for next sample
	for (i=0;i<6;i++)
	{
		prev_alpha[i]=smoothed_alpha[i];
	}
	return;
}

void FTNoIR_Filter_EWMA2::getFilterFullName(QString *strToBeFilled)
{
	*strToBeFilled = filterFullName;
};


void FTNoIR_Filter_EWMA2::getFilterShortName(QString *strToBeFilled)
{
	*strToBeFilled = filterShortName;
};


void FTNoIR_Filter_EWMA2::getFilterDescription(QString *strToBeFilled)
{
	*strToBeFilled = filterDescription;
};

bool FTNoIR_Filter_EWMA2::setParameterValue(const int index, const float newvalue)
{
	if ((index >= 0) && (index < parameterValueAsFloat.size()))
	{
		parameterValueAsFloat[index]=std::min(std::max(newvalue,parameterRange[index].first),parameterRange[index].second);
//		updateParameterString(index);

		if (index==kMinSmoothing || index==kMaxSmoothing)
		{
			smoothing_frames_range=parameterValueAsFloat[kMaxSmoothing]-parameterValueAsFloat[kMinSmoothing];
		}
		return true;
	}
	else
	{
		return false;
	}
};


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Filter object.

// Export both decorated and undecorated names.
//   GetFilter     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetFilter@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetFilter=_GetFilter@0")

FTNOIR_FILTER_BASE_EXPORT FILTERHANDLE __stdcall GetFilter()
{
	return new FTNoIR_Filter_EWMA2;
}
