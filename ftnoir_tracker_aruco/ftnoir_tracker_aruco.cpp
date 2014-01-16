/* Copyright (c) 2013 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_base/ftnoir_tracker_base.h"
#include "ftnoir_tracker_aruco.h"
#include "ui_aruco-trackercontrols.h"
#include "facetracknoir/global-settings.h"
#include <cmath>
#include <QMutexLocker>
#include <aruco.h>
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>
#include <vector>
#include <cstdio>

#if defined(_WIN32)
#   undef NOMINMAX
#   define NOMINMAX
#   define NO_DSHOW_STRSAFE
#   include <dshow.h>
#else
#   include <unistd.h>
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

Tracker::Tracker() : stop(false), layout(nullptr), videoWidget(nullptr)
{
}

Tracker::~Tracker()
{
    stop = true;
    wait();
	if (videoWidget)
		delete videoWidget;
    if(layout)
        delete layout;
    qDebug() << "releasing camera, brace for impact";
    camera.release();
    qDebug() << "all done!";
}

void Tracker::StartTracker(QFrame* videoframe)
{
    videoframe->show();
    videoWidget = new ArucoVideoWidget(videoframe);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(videoWidget);
    if (videoframe->layout())
        delete videoframe->layout();
    videoframe->setLayout(layout);
    videoWidget->show();
    start();
    for (int i = 0; i < 6; i++)
        pose[i] = 0;
    this->layout = layout;
}

#define HT_PI 3.1415926535

void Tracker::run()
{
    int res = s.resolution;
    if (res < 0 || res >= (int)(sizeof(resolution_choices) / sizeof(resolution_tuple)))
        res = 0;
    resolution_tuple r = resolution_choices[res];
    int fps;
    switch (static_cast<int>(s.force_fps))
    {
    default:
    case 0:
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
    case 4:
        fps = 180;
        break;
    }
    camera = cv::VideoCapture(s.camera_index);
    if (r.width)
    {
        camera.set(CV_CAP_PROP_FRAME_WIDTH, r.width);
        camera.set(CV_CAP_PROP_FRAME_HEIGHT, r.height);
    }
    if (fps)
        camera.set(CV_CAP_PROP_FPS, fps);
    
    aruco::MarkerDetector detector;
    detector.setDesiredSpeed(3);

    cv::Rect last_roi(65535, 65535, 0, 0);

    cv::Mat color, color_, grayscale, rvec, tvec;

    const double stateful_coeff = 0.88;
    
    if (!camera.isOpened())
    {
        fprintf(stderr, "aruco tracker: can't open camera\n");
        return;
    }

    auto freq = cv::getTickFrequency();
    auto last_time = cv::getTickCount();
    int cur_fps = 0;
    int last_fps = 0;
    cv::Point2f last_centroid;
    bool first = true;

    while (!stop)
    {
        if (!camera.read(color_))
            continue;
        auto tm = cv::getTickCount();
        color_.copyTo(color);
        if (s.red_only)
        {
            cv::Mat channel[3];
            cv::split(color, channel);
            grayscale = channel[2];
        } else
            cv::cvtColor(color, grayscale, cv::COLOR_BGR2GRAY);

        const int scale = frame.cols > 480 ? 2 : 1;
        detector.setThresholdParams(scale > 1 ? 11 : 7, 4);

        const float focal_length_w = 0.5 * grayscale.cols / tan(0.5 * s.fov * HT_PI / 180);
        const float focal_length_h = 0.5 * grayscale.rows / tan(0.5 * s.fov * grayscale.rows / grayscale.cols * HT_PI / 180.0);
        cv::Mat intrinsics = cv::Mat::eye(3, 3, CV_32FC1);
        intrinsics.at<float> (0, 0) = focal_length_w;
        intrinsics.at<float> (1, 1) = focal_length_h;
        intrinsics.at<float> (0, 2) = grayscale.cols/2;
        intrinsics.at<float> (1, 2) = grayscale.rows/2;

        cv::Mat dist_coeffs = cv::Mat::zeros(5, 1, CV_32FC1);

        std::vector< aruco::Marker > markers;

        const double size_min = 0.04;
        const double size_max = 0.38;

        if (last_roi.width > 0 &&
                (detector.detect(grayscale(last_roi), markers, cv::Mat(), cv::Mat(), -1, false),
                 markers.size() == 1 && markers[0].size() == 4))
        {
            detector.setMinMaxSize(std::max(0.01, size_min * grayscale.cols / last_roi.width),
                                   std::min(1.0, size_max * grayscale.cols / last_roi.width));
            auto& m = markers.at(0);
            for (int i = 0; i < 4; i++)
            {
                auto& p = m.at(i);
                p.x += last_roi.x;
                p.y += last_roi.y;
            }
        }
        else
        {
            detector.setMinMaxSize(size_min, size_max);
            detector.detect(grayscale, markers, cv::Mat(), cv::Mat(), -1, false);
        }

        if (markers.size() == 1 && markers[0].size() == 4) {
            const auto& m = markers.at(0);
            for (int i = 0; i < 4; i++)
                cv::line(color, m[i], m[(i+1)%4], cv::Scalar(0, 0, 255), scale, 8);
        }

        auto time = cv::getTickCount();

        if ((long) (time / freq) != (long) (last_time / freq))
        {
            last_fps = cur_fps;
            cur_fps = 0;
            last_time = time;
        }

        cur_fps++;

        char buf[128];

        frame = color.clone();

        ::sprintf(buf, "Hz: %d", last_fps);
        cv::putText(frame, buf, cv::Point(10, 32), cv::FONT_HERSHEY_PLAIN, scale, cv::Scalar(0, 255, 0), scale);
        ::sprintf(buf, "Jiffies: %ld", (long) (10000 * (time - tm) / freq));
        cv::putText(frame, buf, cv::Point(10, 54), cv::FONT_HERSHEY_PLAIN, scale, cv::Scalar(80, 255, 0), scale);
        
        if (markers.size() == 1 && markers[0].size() == 4) {
            const auto& m = markers.at(0);
            const float size = 40;
            
            const double p = s.marker_pitch;
            const double sq = sin(p * HT_PI / 180);
            const double cq = cos(p * HT_PI / 180);

            cv::Mat obj_points(4,3,CV_32FC1);
            obj_points.at<float>(1,0)=-size + s.headpos_x;
            obj_points.at<float>(1,1)=-size * cq + s.headpos_y;
            obj_points.at<float>(1,2)=-size * sq + s.headpos_z;
            obj_points.at<float>(2,0)=size + s.headpos_x;
            obj_points.at<float>(2,1)=-size * cq + s.headpos_y;
            obj_points.at<float>(2,2)=-size * sq + s.headpos_z;
            obj_points.at<float>(3,0)=size + s.headpos_x;
            obj_points.at<float>(3,1)=size * cq + s.headpos_y;
            obj_points.at<float>(3,2)=size * sq + s.headpos_z;
            obj_points.at<float>(0,0)=-size + s.headpos_x;
            obj_points.at<float>(0,1)=size * cq + s.headpos_y;
            obj_points.at<float>(0,2)=size * sq + s.headpos_z;

            last_roi = cv::Rect(65535, 65535, 0, 0);

            for (int i = 0; i < 4; i++)
            {
                auto foo = m.at(i);
                last_roi.x = std::min<int>(foo.x, last_roi.x);
                last_roi.y = std::min<int>(foo.y, last_roi.y);
                last_roi.width = std::max<int>(foo.x, last_roi.width);
                last_roi.height = std::max<int>(foo.y, last_roi.height);
            }
            {
                last_roi.width -= last_roi.x;
                last_roi.height -= last_roi.y;
                last_roi.x -= last_roi.width * stateful_coeff;
                last_roi.y -= last_roi.height * stateful_coeff;
                last_roi.width *= stateful_coeff * 3;
                last_roi.height *= stateful_coeff * 3;
                last_roi.x = std::max<int>(0, last_roi.x);
                last_roi.y = std::max<int>(0, last_roi.y);
                last_roi.width = std::min<int>(grayscale.cols - last_roi.x, last_roi.width);
                last_roi.height = std::min<int>(grayscale.rows - last_roi.y, last_roi.height);
            }

            cv::solvePnP(obj_points, m, intrinsics, dist_coeffs, rvec, tvec, !first, cv::ITERATIVE);
            first = false;
            cv::Mat rotation_matrix = cv::Mat::zeros(3, 3, CV_64FC1);
            cv::Mat junk1(3, 3, CV_64FC1), junk2(3, 3, CV_64FC1);
            cv::Rodrigues(rvec, rotation_matrix);

            {
                cv::Vec3d euler = cv::RQDecomp3x3(rotation_matrix, junk1, junk2);

                if (fabs(euler[0]) + fabs(s.marker_pitch) > 60)
                {
                    first = true;
                    qDebug() << "reset levmarq due to pitch breakage";
                }

                QMutexLocker lck(&mtx);

                for (int i = 0; i < 3; i++)
                    pose[i] = tvec.at<double>(i);

                pose[Yaw] = euler[1];
                pose[Pitch] = -euler[0];
                pose[Roll] = euler[2];
            }

            std::vector<cv::Point2f> repr2;
            std::vector<cv::Point3f> centroid;
            centroid.push_back(cv::Point3f(0, 0, 0));
            cv::projectPoints(centroid, rvec, tvec, intrinsics, dist_coeffs, repr2);

            {
                auto s = cv::Scalar(255, 0, 255);
                cv::circle(frame, repr2.at(0), 4, s, -1);
            }

            last_centroid = repr2[0];
        }
        else
        {
            last_roi = cv::Rect(65535, 65535, 0, 0);
            first = true;
        }

        if (frame.rows > 0)
            videoWidget->update_image(frame);
    }
}

void Tracker::GetHeadPoseData(double *data)
{
    QMutexLocker lck(&mtx);
    
    if (s.eyaw)
        data[Yaw] = pose[Yaw];
    if (s.epitch)
        data[Pitch] = pose[Pitch];
    if (s.eroll)
        data[Roll] = pose[Roll];
    if (s.ex)
        data[TX] = pose[TX] * .1;
    if (s.ey)
        data[TY] = pose[TY] * .1;
    if (s.ez)
        data[TZ] = pose[TZ] * .1;
}

class TrackerDll : public Metadata
{
	// ITrackerDll interface
	void getFullName(QString *strToBeFilled);
	void getShortName(QString *strToBeFilled);
	void getDescription(QString *strToBeFilled);
	void getIcon(QIcon *icon);
};

//-----------------------------------------------------------------------------
void TrackerDll::getFullName(QString *strToBeFilled)
{
    *strToBeFilled = "aruco";
}

void TrackerDll::getShortName(QString *strToBeFilled)
{
	*strToBeFilled = "aruco";
}

void TrackerDll::getDescription(QString *strToBeFilled)
{
	*strToBeFilled = "";
}

void TrackerDll::getIcon(QIcon *icon)
{
    *icon = QIcon(":/images/aruco.png");
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
    tracker = nullptr;
	ui.setupUi(this);
    setAttribute(Qt::WA_NativeWindow, true);
    tie_setting(s.camera_index, ui.cameraName);
	tie_setting(s.resolution, ui.resolution);
    tie_setting(s.force_fps, ui.cameraFPS);
    tie_setting(s.fov, ui.cameraFOV);
    tie_setting(s.eyaw, ui.rx);
    tie_setting(s.epitch, ui.ry);
    tie_setting(s.eroll, ui.rz);
    tie_setting(s.ex, ui.tx);
    tie_setting(s.ey, ui.ty);
    tie_setting(s.ez, ui.tz);
    tie_setting(s.headpos_x, ui.cx);
    tie_setting(s.headpos_y, ui.cy);
    tie_setting(s.headpos_z, ui.cz);
    tie_setting(s.red_only, ui.red_only);
    tie_setting(s.marker_pitch, ui.marker_pitch);
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    ui.cameraName->addItems(get_camera_names());
}

void TrackerControls::doOK()
{
    s.b->save();
    if (tracker)
        tracker->reload();
	this->close();
}

void TrackerControls::doCancel()
{
    s.b->revert();
    this->close();
}
