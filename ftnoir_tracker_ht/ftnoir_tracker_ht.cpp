#include "stdafx.h"
#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "headtracker-ftnoir.h"
#include "ftnoir_tracker_ht.h"
#include "ftnoir_tracker_ht_dll.h"
#include "ui_ht-trackercontrols.h"
#include "facetracknoir/global-settings.h"
#include <cmath>

#if defined(_WIN32)
#include <dshow.h>
#else
#include <unistd.h>
#endif

// delicious copypasta
static QList<QString> get_camera_names(void) {
    QList<QString> ret;
#if defined(_WIN32)
	// Create the System Device Enumerator.
	HRESULT hr;
	ICreateDevEnum *pSysDevEnum = NULL;
	hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
	if (FAILED(hr))
	{
		return ret;
	}
	// Obtain a class enumerator for the video compressor category.
	IEnumMoniker *pEnumCat = NULL;
	hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);

	if (hr == S_OK) {
		// Enumerate the monikers.
		IMoniker *pMoniker = NULL;
		ULONG cFetched;
		while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK) {
			IPropertyBag *pPropBag;
			hr = pMoniker->BindToStorage(0, 0, IID_IPropertyBag, (void **)&pPropBag);
			if (SUCCEEDED(hr))	{
				// To retrieve the filter's friendly name, do the following:
				VARIANT varName;
				VariantInit(&varName);
				hr = pPropBag->Read(L"FriendlyName", &varName, 0);
				if (SUCCEEDED(hr))
				{
					// Display the name in your UI somehow.
					QString str((QChar*)varName.bstrVal, wcslen(varName.bstrVal));
					ret.append(str);
				}
				VariantClear(&varName);

				////// To create an instance of the filter, do the following:
				////IBaseFilter *pFilter;
				////hr = pMoniker->BindToObject(NULL, NULL, IID_IBaseFilter,
				////	(void**)&pFilter);
				// Now add the filter to the graph. 
				//Remember to release pFilter later.
				pPropBag->Release();
			}
			pMoniker->Release();
		}
		pEnumCat->Release();
	}
	pSysDevEnum->Release();
#else
    for (int i = 0; i < 16; i++) {
        char buf[128];
        sprintf(buf, "/dev/video%d", i);
        if (access(buf, R_OK | W_OK) == 0) {
            ret.append(buf);
        } else {
            continue;
        }
    }
#endif
    return ret;
}

typedef struct {
	int width;
	int height;
} resolution_tuple;

static resolution_tuple resolution_choices[] = {
	{ 640, 480 },
	{ 320, 240 },
	{ 320, 200 },
	{ 0, 0 }
};

static void load_settings(ht_config_t* config, Tracker* tracker)
{
	QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );

	iniFile.beginGroup( "HT-Tracker" );
    config->classification_delay = 500;
    config->field_of_view = iniFile.value("fov", 52).toFloat();
	config->pyrlk_pyramids = 2;
    config->pyrlk_win_size_w = config->pyrlk_win_size_h = 21;
    config->max_keypoints = 300;
    config->keypoint_distance = 4.3;
    //config->force_width = 640;
    //config->force_height = 480;
    config->force_fps = iniFile.value("fps", 0).toInt();
    config->camera_index = iniFile.value("camera-index", -1).toInt();
    config->ransac_num_iters = 100;
    config->ransac_max_reprojection_error = 6.5;
    config->ransac_max_inlier_error = 6.5;
    config->ransac_abs_max_mean_error = 15;
    config->ransac_max_mean_error = 4.5;
    config->debug = 0;
    config->ransac_min_features = 0.76;
    int res = iniFile.value("resolution", 0).toInt();
    if (res < 0 || res >= (int)(sizeof(resolution_choices) / sizeof(resolution_tuple)))
		res = 0;
	resolution_tuple r = resolution_choices[res];
	config->force_width = r.width;
    config->force_height = r.height;
    config->flandmark_delay = 250;
    qDebug() << "width" << r.width << "height" << r.height;
	if (tracker)
	{
		tracker->enableRX = iniFile.value("enable-rx", true).toBool();
		tracker->enableRY = iniFile.value("enable-ry", true).toBool();
		tracker->enableRZ = iniFile.value("enable-rz", true).toBool();
		tracker->enableTX = iniFile.value("enable-tx", true).toBool();
		tracker->enableTY = iniFile.value("enable-ty", true).toBool();
		tracker->enableTZ = iniFile.value("enable-tz", true).toBool();
	}
    
    for (int i = 0; i < 5; i++)
        config->dist_coeffs[i] = iniFile.value(QString("dc%1").arg(i), 0).toDouble();
                
	iniFile.endGroup();
}

Tracker::Tracker() : lck_shm(HT_SHM_NAME, HT_MUTEX_NAME, sizeof(ht_shm_t))
{
	videoWidget = NULL;
	layout = NULL;
	enableRX = enableRY = enableRZ = enableTX = enableTY = enableTZ = true;
    shm = (ht_shm_t*) lck_shm.mem;
    shm->terminate = 0;
	load_settings(&shm->config, this);
    shm->result.filled = false;
}

Tracker::~Tracker()
{
    if (shm) {
        shm->terminate = true;
        subprocess.waitForFinished(5000);
    }
    subprocess.kill();
    if (shm)
        shm->terminate = true;
	if (layout)
		delete layout;
	if (videoWidget)
		delete videoWidget;
}

void Tracker::StartTracker(QFrame* videoframe)
{
    videoframe->show();
    videoWidget = new HTVideoWidget(videoframe);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(videoWidget);
    if (videoframe->layout())
        delete videoframe->layout();
    videoframe->setLayout(layout);
    videoWidget->show();
    this->layout = layout;
    load_settings(&shm->config, this);
    shm->frame.channels = shm->frame.width = shm->frame.height = 0;
    shm->pause = shm->terminate = shm->running = false;
    shm->timer = 0;
    subprocess.setWorkingDirectory(QCoreApplication::applicationDirPath() + "/tracker-ht");
#if defined(_WIN32)
    subprocess.start("\"" + QCoreApplication::applicationDirPath() + "/tracker-ht/headtracker-ftnoir" + "\"");
#else
    subprocess.start(QCoreApplication::applicationDirPath() + "/tracker-ht/headtracker-ftnoir");
#endif
}

bool Tracker::GiveHeadPoseData(double *data)
{
	bool ret = false;

    lck_shm.lock();
    shm->timer = 0;
    if (shm->frame.width > 0)
    {
        videoWidget->update_image(shm->frame.frame, shm->frame.width, shm->frame.height);
        //memcpy(foo, shm->frame.frame, shm->frame.width * shm->frame.height * 3);
        shm->frame.width = 0;
    }
    if (shm->result.filled) {
        if (enableRX)
            data[Yaw] = shm->result.rotx;
        if (enableRY) {
            data[Pitch] = shm->result.roty;
		}
        if (enableRZ) {
            data[Roll] = shm->result.rotz;
        }
        if (enableTX)
            data[TX] = shm->result.tx;
        if (enableTY)
            data[TY] = shm->result.ty;
        if (enableTZ)
            data[TZ] = shm->result.tz;
        ret = true;
        if (fabs(data[Yaw]) > 60 || fabs(data[Pitch]) > 50 || fabs(data[Roll]) > 40)
        {
            shm->pause = true;
        }
    } else {
        shm->pause = false;
    }
    lck_shm.unlock();

	return ret;
}

//-----------------------------------------------------------------------------
void TrackerDll::getFullName(QString *strToBeFilled)
{
    *strToBeFilled = "HT 1.0";
}

void TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = "HT";
}

void TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = "";
}

void TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/ht.png");
}


//-----------------------------------------------------------------------------
//#pragma comment(linker, "/export:GetTrackerDll=_GetTrackerDll@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT Metadata* CALLING_CONVENTION GetMetadata()
{
	return new TrackerDll;
}

//#pragma comment(linker, "/export:GetTracker=_GetTracker@0")

extern "C" FTNOIR_TRACKER_BASE_EXPORT ITracker* CALLING_CONVENTION GetConstructor()
{
    return new Tracker;
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

TrackerControls::TrackerControls()
{
	ui.setupUi(this);
    setAttribute(Qt::WA_NativeWindow, true);
	connect(ui.cameraName, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.cameraFPS, SIGNAL(currentIndexChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.cameraFOV, SIGNAL(valueChanged(double)), this, SLOT(settingChanged(double)));
	connect(ui.rx, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.ry, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.rz, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.tx, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.ty, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.tz, SIGNAL(stateChanged(int)), this, SLOT(settingChanged(int)));
	connect(ui.buttonCancel, SIGNAL(clicked()), this, SLOT(doCancel()));
	connect(ui.buttonOK, SIGNAL(clicked()), this, SLOT(doOK()));
    //connect(ui.buttonSettings, SIGNAL(clicked()), this, SLOT(cameraSettings()));
    loadSettings();
	settingsDirty = false;
}

TrackerControls::~TrackerControls()
{
}

void TrackerControls::showEvent(QShowEvent *)
{
}

void TrackerControls::Initialize(QWidget*)
{
    loadSettings();
	show();
}

void TrackerControls::loadSettings()
{
	ui.cameraName->clear();
	QList<QString> names = get_camera_names();
	names.prepend("Any available");
	ui.cameraName->addItems(names);
	QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );
	iniFile.beginGroup( "HT-Tracker" );
	ui.cameraName->setCurrentIndex(iniFile.value("camera-index", -1).toInt() + 1);
    ui.cameraFOV->setValue(iniFile.value("fov", 52).toFloat());
	int fps;
	switch (iniFile.value("fps", 0).toInt())
	{
	default:
	case 0:
		fps = 0;
		break;
	case 30:
		fps = 1;
		break;
	case 60:
		fps = 2;
		break;
	case 120:
		fps = 3;
		break;
	}
	ui.cameraFPS->setCurrentIndex(fps);
	ui.rx->setCheckState(iniFile.value("enable-rx", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.ry->setCheckState(iniFile.value("enable-ry", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.rz->setCheckState(iniFile.value("enable-rz", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.tx->setCheckState(iniFile.value("enable-tx", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.ty->setCheckState(iniFile.value("enable-ty", true).toBool() ? Qt::Checked : Qt::Unchecked);
	ui.tz->setCheckState(iniFile.value("enable-tz", true).toBool() ? Qt::Checked : Qt::Unchecked);
    ui.resolution->setCurrentIndex(iniFile.value("resolution", 0).toInt());
    
    ui.doubleSpinBox->setValue(iniFile.value("dc0").toDouble());
    ui.doubleSpinBox_2->setValue(iniFile.value("dc1").toDouble());
    ui.doubleSpinBox_3->setValue(iniFile.value("dc2").toDouble());
    ui.doubleSpinBox_4->setValue(iniFile.value("dc3").toDouble());
    ui.doubleSpinBox_5->setValue(iniFile.value("dc4").toDouble());
    
	iniFile.endGroup();
	settingsDirty = false;
}

void TrackerControls::save()
{
	QSettings settings("opentrack");
	QString currentFile = settings.value( "SettingsFile", QCoreApplication::applicationDirPath() + "/settings/default.ini" ).toString();
	QSettings iniFile( currentFile, QSettings::IniFormat );

	iniFile.beginGroup( "HT-Tracker" );
	iniFile.setValue("fov", ui.cameraFOV->value());
	int fps;
	switch (ui.cameraFPS->currentIndex())
	{
	case 0:
	default:
		fps = 0;
		break;
	case 1:
		fps = 30;
		break;
	case 2:
		fps = 60;
		break;
	case 3:
		fps = 120;
		break;
	}
	iniFile.setValue("fps", fps);
	iniFile.setValue("camera-index", ui.cameraName->currentIndex() - 1);
	iniFile.setValue("enable-rx", ui.rx->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-ry", ui.ry->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-rz", ui.rz->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-tx", ui.tx->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-ty", ui.ty->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("enable-tz", ui.tz->checkState() != Qt::Unchecked ? true : false);
	iniFile.setValue("resolution", ui.resolution->currentIndex());
    
    iniFile.setValue("dc0", ui.doubleSpinBox->value());
    iniFile.setValue("dc1", ui.doubleSpinBox_2->value());
    iniFile.setValue("dc2", ui.doubleSpinBox_3->value());
    iniFile.setValue("dc3", ui.doubleSpinBox_4->value());
    iniFile.setValue("dc4", ui.doubleSpinBox_5->value());
    
	iniFile.endGroup();
	settingsDirty = false;
}

void TrackerControls::doOK()
{
	save();
	this->close();
}

void TrackerControls::doCancel()
{
	if (settingsDirty) {
		int ret = QMessageBox::question ( this,
										  "Settings have changed",
										  "Do you want to save the settings?",
										  QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel,
										  QMessageBox::Discard );

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
