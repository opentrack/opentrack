#include "ftnoir_tracker_joystick.h"
#include "facetracknoir/global-settings.h"

static BOOL CALLBACK EnumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext )
{
    auto self = ( TrackerControls* )pContext;

    self->guids.push_back(pdidInstance->guidInstance);

    return DIENUM_CONTINUE;
}

TrackerControls::TrackerControls() :
QWidget()
{
	ui.setupUi( this );

	// Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    connect(ui.spinBox, SIGNAL(valueChanged(int)), this, SLOT(settingChanged(int)));
    connect(ui.comboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
    connect(ui.comboBox_2, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
    connect(ui.comboBox_3, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
    connect(ui.comboBox_4, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
    connect(ui.comboBox_5, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
    connect(ui.comboBox_6, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));

    {
        auto hr = CoInitialize( nullptr );
        LPDIRECTINPUT8 g_pDI = nullptr;

        if( FAILED( hr = DirectInput8Create( GetModuleHandle( NULL ), DIRECTINPUT_VERSION,
                                             IID_IDirectInput8, ( VOID** )&g_pDI, NULL ) ) )
            goto fin;

        if( FAILED( hr = g_pDI->EnumDevices( DI8DEVCLASS_GAMECTRL,
                                             EnumJoysticksCallback,
                                             this,
                                             DIEDFL_ATTACHEDONLY )))
            goto fin;

fin:
        if (g_pDI)
            g_pDI->Release();
    }

	loadSettings();
}

//
// Destructor for server-dialog
//
TrackerControls::~TrackerControls() {
}

void TrackerControls::Initialize(QWidget *parent) {
	QPoint offsetpos(100, 100);
	if (parent) {
		this->move(parent->pos() + offsetpos);
	}
	show();
}

void TrackerControls::doOK() {
	save();
	this->close();
}

void TrackerControls::showEvent ( QShowEvent * ) {
	loadSettings();
}

void TrackerControls::doCancel() {
	if (settingsDirty) {
		int ret = QMessageBox::question ( this, "Settings have changed", "Do you want to save the settings?", QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel, QMessageBox::Discard );

        switch (ret) {
			case QMessageBox::Save:
				save();
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

void TrackerControls::loadSettings() {

	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    QComboBox* boxen[] = {
        ui.comboBox,
        ui.comboBox_2,
        ui.comboBox_3,
        ui.comboBox_4,
        ui.comboBox_5,
        ui.comboBox_6,
    };

    iniFile.beginGroup ( "tracker-joy" );
    for (int i = 0; i < 6; i++)
    {
        boxen[i]->setCurrentIndex(iniFile.value(QString("axis-%1").arg(i), 0).toInt());
    }
    ui.spinBox->setValue(iniFile.value("joyid", -1).toInt());
	iniFile.endGroup ();

	settingsDirty = false;
}

void TrackerControls::save() {
	QSettings settings("opentrack");	// Registry settings (in HK_USER)

	QString currentFile = settings.value ( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );		// Application settings (in INI-file)

    QComboBox* boxen[] = {
        ui.comboBox,
        ui.comboBox_2,
        ui.comboBox_3,
        ui.comboBox_4,
        ui.comboBox_5,
        ui.comboBox_6,
    };

    iniFile.beginGroup ( "tracker-joy" );
    for (int i = 0; i < 6; i++)
    {
        iniFile.setValue(QString("axis-%1").arg(i), boxen[i]->currentIndex());
    }
	iniFile.endGroup ();

    if(tracker)
    {
        tracker->reload();
    }

	settingsDirty = false;
}
////////////////////////////////////////////////////////////////////////////////
// Factory function that creates instances if the Tracker-settings dialog object.

// Export both decorated and undecorated names.
//   GetTrackerDialog     - Undecorated name, which can be easily used with GetProcAddress
//                          Win32 API function.
//   _GetTrackerDialog@0  - Common name decoration for __stdcall functions in C language.
//#pragma comment(linker, "/export:GetTrackerDialog=_GetTrackerDialog@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITrackerDialog* CALLING_CONVENTION GetDialog( )
{
    return new TrackerControls;
}
