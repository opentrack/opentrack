#include "ftnoir_tracker_sm.h"

using namespace sm::faceapi;
using namespace sm::faceapi::qt;

FTNoIR_Tracker_SM::FTNoIR_Tracker_SM()
{
	//allocate memory for the parameters
	parameterValueAsFloat.clear();
	parameterRange.clear();

	// Add the parameters to the list
	parameterRange.append(std::pair<float,float>(1000.0f,9999.0f));
	parameterValueAsFloat.append(0.0f);
	setParameterValue(kPortAddress,5551.0f);

	newHeadPose.x = 0.0f;
	newHeadPose.y = 0.0f;
	newHeadPose.z = 0.0f;
	newHeadPose.yaw   = 0.0f;
	newHeadPose.pitch = 0.0f;
	newHeadPose.roll  = 0.0f;
}

FTNoIR_Tracker_SM::~FTNoIR_Tracker_SM()
{
}

void FTNoIR_Tracker_SM::Release()
{
    delete this;
}

void FTNoIR_Tracker_SM::Initialize()
{
	qDebug() << "FTNoIR_Tracker_SM::Initialize says: Starting ";
	loadSettings();

	try {
		// Initialize the faceAPI Qt library
		sm::faceapi::qt::initialize();
		smLoggingSetFileOutputEnable( false );

		// Initialize the API
		faceapi_scope = new APIScope();

		//if (APIScope::internalQtGuiIsDisabled()){
		//	QMessageBox::warning(0,"faceAPI Error","Something Bad",QMessageBox::Ok,QMessageBox::NoButton);
		//}

		// Create head-tracking engine v2 using first detected webcam
		CameraInfo::registerType(SM_API_CAMERA_TYPE_WDM);
		_engine = QSharedPointer<HeadTrackerV2>(new HeadTrackerV2());	

		// starts the faceapi engine
		_engine->start();
	} 
	catch (sm::faceapi::Error &e)
	{
		/* ERROR with camera */
		QMessageBox::warning(0,"faceAPI Error",e.what(),QMessageBox::Ok,QMessageBox::NoButton);
	}

	return;
}

void FTNoIR_Tracker_SM::StartTracker()
{
	return;
}

void FTNoIR_Tracker_SM::GiveHeadPoseData(THeadPoseData *data)
{
	data->x = newHeadPose.x;
	data->y = newHeadPose.y;
	data->z = newHeadPose.z;
	data->yaw = newHeadPose.yaw;
	data->pitch = newHeadPose.pitch;
	data->roll = newHeadPose.roll;
	return;
}

bool FTNoIR_Tracker_SM::setParameterValue(const int index, const float newvalue)
{
	if ((index >= 0) && (index < parameterValueAsFloat.size()))
	{
		//
		// Limit the new value, using the defined range.
		//
		if (newvalue < parameterRange[index].first) {
			parameterValueAsFloat[index] = parameterRange[index].first;
		}
		else {
			if (newvalue > parameterRange[index].second) {
				parameterValueAsFloat[index] = parameterRange[index].second;
			}
			else {
				parameterValueAsFloat[index] = newvalue;
			}
		}

//		updateParameterString(index);
		return true;
	}
	else
	{
		return false;
	}
};

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker_SM::loadSettings() {

	qDebug() << "FTNoIR_Tracker_SM::loadSettings says: Starting ";
	QSettings settings("Abbequerque Inc.", "FaceTrackNoIR");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/Settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

	qDebug() << "FTNoIR_Tracker_SM::loadSettings says: iniFile = " << currentFile;

	iniFile.beginGroup ( "FTNClient" );
	setParameterValue(kPortAddress, (float) iniFile.value ( "PortNumber", 5550 ).toInt());
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

FTNOIR_TRACKER_BASE_EXPORT TRACKERHANDLE __stdcall GetTracker()
{
	return new FTNoIR_Tracker_SM;
}
