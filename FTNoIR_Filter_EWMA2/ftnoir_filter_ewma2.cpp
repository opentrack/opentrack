#include "ftnoir_filter_base.h"
#include "math.h"
#include <QDebug>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// EWMA Filter: Exponentially Weighted Moving Average filter with dynamic smoothing parameter
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class FTNoIR_Filter_EWMA2 : public IFilter
{
public:
	FTNoIR_Filter_EWMA2();
	~FTNoIR_Filter_EWMA2();

	void Release();
    void Initialize();
    void StartFilter();
	void FilterHeadPoseData(THeadPoseData *current_camera_position, THeadPoseData *target_camera_position, THeadPoseData *new_camera_position, bool newTarget);

	void getFilterFullName(QString *strToBeFilled);
	void getFilterShortName(QString *strToBeFilled);
	void getFilterDescription(QString *strToBeFilled);

	bool setParameterValue(const int index, const float newvalue);

private:
	THeadPoseData newHeadPose;								// Structure with new headpose

	bool	first_run;
	float	smoothing_frames_range;
	float	alpha_smoothing;
	float	prev_alpha[6];
	float	alpha[6];
	float	smoothed_alpha[6];

	//parameter list for the filter-function(s)
	enum
	{
		kMinSmoothing=0,
		kMaxSmoothing,
		kSmoothingScaleCurve,
		kNumFilterParameters								// Indicate number of parameters used
	};

	QString filterFullName;									// Filters' name and description
	QString filterShortName;
	QString filterDescription;

	QList<float>					parameterValueAsFloat;
	QList<std::pair<float,float>>	parameterRange;
	QList<float>					parameterSteps;
	QList<QString>					parameterNameAsString;
	QList<QString>					parameterValueAsString;
	QList<QString>					parameterUnitsAsString;
};

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
	alpha_smoothing = 0.02f;		//this is a constant for now, might be a parameter later

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
	return;
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
	if (newTarget) {
		for (i=0;i<6;i++)
		{
			alpha[i]=1.0f/(parameterValueAsFloat[kMinSmoothing]+((1.0f-pow(norm_output_delta[i],parameterValueAsFloat[kSmoothingScaleCurve]))*smoothing_frames_range));
			smoothed_alpha[i]=(alpha_smoothing*alpha[i])+((1.0f-alpha_smoothing)*prev_alpha[i]);
		}
	}

	qDebug() << "FTNoIR_Filter_EWMA2::FilterHeadPoseData() smoothing frames = " << smoothing_frames_range;
	qDebug() << "FTNoIR_Filter_EWMA2::FilterHeadPoseData() alpha[3] = " << alpha[3];

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
//		output[i]=(largest_alpha*target[i])+((1.0f-largest_alpha)*prev_output[i]);
		output[i]=(smoothed_alpha[i]*target[i])+((1.0f-smoothed_alpha[i])*prev_output[i]);
	}

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
