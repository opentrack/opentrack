#include "stdafx.h"
#include "headtracker-ftnoir.h"
#include "ftnoir_tracker_ht.h"
#include "ftnoir_tracker_ht_dll.h"
#include "ui_ht-trackercontrols.h"
#include "facetracknoir/plugin-api.hpp"
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
#elif !defined(__APPLE__)
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

void Tracker::load_settings(ht_config_t* config)
{
    int nframes = 0;
    switch (static_cast<int>(s.fps))
    {
    default:
    case 0:
        nframes = 0;
        break;
    case 1:
        nframes = 30;
        break;
    case 2:
        nframes = 60;
        break;
    case 3:
        nframes = 120;
        break;
    case 4:
        nframes = 180;
        break;
    }

    config->classification_delay = 500;
    config->field_of_view = s.fov;
    config->pyrlk_pyramids = 0;
    config->pyrlk_win_size_w = config->pyrlk_win_size_h = 21;
    config->max_keypoints = 150;
    config->keypoint_distance = 6;
    config->force_fps = nframes;
    config->camera_index = s.camera_idx - 1;
    config->ransac_num_iters = 100;
    config->ransac_max_reprojection_error = 10;
    config->ransac_max_inlier_error = 10;
    config->ransac_abs_max_mean_error = 14;
    config->ransac_max_mean_error = 8;
    config->debug = 0;
    config->ransac_min_features = 0.86;
    int res = s.resolution;
    if (res < 0 || res >= (int)(sizeof(resolution_choices) / sizeof(resolution_tuple)))
		res = 0;
	resolution_tuple r = resolution_choices[res];
	config->force_width = r.width;
    config->force_height = r.height;
    config->flandmark_delay = 500;
    for (int i = 0; i < 5; i++)
        config->dist_coeffs[i] = 0;
}

Tracker::Tracker() :
    lck_shm(HT_SHM_NAME, HT_MUTEX_NAME, sizeof(ht_shm_t)),
    shm(reinterpret_cast<ht_shm_t*>(lck_shm.ptr())),
    videoWidget(nullptr),
    layout(nullptr)
{
    shm->terminate = 0;
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
    load_settings(&shm->config);
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

void Tracker::GetHeadPoseData(double *data)
{
    lck_shm.lock();
    shm->timer = 0;
    if (shm->frame.width > 0)
    {
        videoWidget->update_image(shm->frame.frame, shm->frame.width, shm->frame.height);
        //memcpy(foo, shm->frame.frame, shm->frame.width * shm->frame.height * 3);
        shm->frame.width = 0;
    }
    if (shm->result.filled) {
        data[Yaw] = shm->result.rotx;
        data[Pitch] = shm->result.roty;
        data[Roll] = shm->result.rotz;
        data[TX] = shm->result.tx;
        data[TY] = shm->result.ty;
        data[TZ] = shm->result.tz;
    } else {
        shm->pause = false;
    }
    lck_shm.unlock();
}

//-----------------------------------------------------------------------------
void TrackerDll::getFullName(QString *strToBeFilled)
{
    *strToBeFilled = "HT face tracker";
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

extern "C" OPENTRACK_EXPORT Metadata* GetMetadata()
{
	return new TrackerDll;
}

extern "C" OPENTRACK_EXPORT ITracker* GetConstructor()
{
    return new Tracker;
}

extern "C" OPENTRACK_EXPORT ITrackerDialog* GetDialog( )
{
    return new TrackerControls;
}

TrackerControls::TrackerControls()
{
	ui.setupUi(this);
    ui.cameraName->clear();
    QList<QString> names = get_camera_names();
    names.prepend("Any available");
    ui.cameraName->addItems(names);
    tie_setting(s.camera_idx, ui.cameraName);
    tie_setting(s.fps, ui.cameraFPS);
    tie_setting(s.fov, ui.cameraFOV);
    tie_setting(s.resolution, ui.resolution);
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
}

void TrackerControls::doOK()
{
    s.b->save();
	this->close();
}

void TrackerControls::doCancel()
{
    s.b->reload();
    this->close();
}
