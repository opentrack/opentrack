#include "ftnoir_tracker_joystick.h"
#include "facetracknoir/global-settings.h"

FTNoIR_Tracker::FTNoIR_Tracker()
{
	bEnableRoll = true;
	bEnablePitch = true;
	bEnableYaw = true;
	bEnableX = true;
	bEnableY = true;
	bEnableZ = true;
}

FTNoIR_Tracker::~FTNoIR_Tracker()
{
}

void FTNoIR_Tracker::StartTracker(QFrame*)
{
    loadSettings();
}

bool FTNoIR_Tracker::GiveHeadPoseData(double *data)
{
	return true;
}

//
// Load the current Settings from the currently 'active' INI-file.
//
void FTNoIR_Tracker::loadSettings() {

	QSettings settings("opentrack");	// Registry settings (in HK_USER)
	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)
    iniFile.beginGroup ( "tracker-joy" );
	bEnableRoll = iniFile.value ( "EnableRoll", 1 ).toBool();
	bEnablePitch = iniFile.value ( "EnablePitch", 1 ).toBool();
	bEnableYaw = iniFile.value ( "EnableYaw", 1 ).toBool();
	bEnableX = iniFile.value ( "EnableX", 1 ).toBool();
	bEnableY = iniFile.value ( "EnableY", 1 ).toBool();
	bEnableZ = iniFile.value ( "EnableZ", 1 ).toBool();
	iniFile.endGroup ();
}


////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker object.

// Export both decorated and undecorated names.
//   GetTracker     - Undecorated name, which can be easily used with GetProcAddress
//                Win32 API function.
//   _GetTracker@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new FTNoIR_Tracker;
}
