#include "stdafx.h"
#include "headtracker-ftnoir.h"
#include "ftnoir_tracker_ht.h"
#include "ftnoir_tracker_ht_dll.h"
#include "ui_ht-trackercontrols.h"
#include "opentrack/plugin-api.hpp"
#include <cmath>
#include "opentrack/camera-names.hpp"

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
    config->max_keypoints = 150;
    config->keypoint_distance = 3.4;
    config->force_fps = nframes;
    config->camera_index = camera_name_to_index(s.camera_name);

    config->ransac_max_reprojection_error = 8;
    config->ransac_max_inlier_error = 8;

    config->pyrlk_pyramids = 0;
    config->pyrlk_win_size_w = config->pyrlk_win_size_h = 21;

    config->ransac_max_mean_error = 999;
    config->ransac_abs_max_mean_error = 999;

    config->debug = 0;
    config->ransac_min_features = 0.85;
    config->ransac_num_iters = 300;

    int res = s.resolution;
    if (res < 0 || res >= (int)(sizeof(resolution_choices) / sizeof(resolution_tuple)))
		res = 0;
	resolution_tuple r = resolution_choices[res];
	config->force_width = r.width;
    config->force_height = r.height;
    config->flandmark_delay = 50;
    for (int i = 0; i < 5; i++)
        config->dist_coeffs[i] = 0;
}

Tracker::Tracker() :
    ht(nullptr),
    ypr {0,0,0, 0,0,0},
    videoWidget(nullptr),
    layout(nullptr),
    should_stop(false)
{
}

Tracker::~Tracker()
{
    should_stop = true;
    wait();
    ht_free_context(ht);
	if (layout)
		delete layout;
	if (videoWidget)
		delete videoWidget;
}

void Tracker::start_tracker(QFrame* videoframe)
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

    load_settings(&conf);
    ht = ht_make_context(&conf, nullptr);
    start();
}

void Tracker::run()
{
    while (!should_stop)
    {
        ht_result_t euler;
        euler.filled = false;
        {
            QMutexLocker l(&camera_mtx);

            if (!ht_cycle(ht, &euler))
                break;
        }
        if (euler.filled)
        {
            QMutexLocker l(&ypr_mtx);
            ypr[TX] = euler.tx;
            ypr[TY] = euler.ty;
            ypr[TZ] = euler.tz;
            ypr[Yaw] = euler.rotx;
            ypr[Pitch] = euler.roty;
            ypr[Roll] = euler.rotz;
        }
        {
            const cv::Mat frame_ = ht_get_bgr_frame(ht);
            if (frame_.cols <= HT_MAX_VIDEO_WIDTH && frame_.rows <= HT_MAX_VIDEO_HEIGHT && frame_.channels() <= HT_MAX_VIDEO_CHANNELS)
            {
                QMutexLocker l(&frame_mtx);

                const int cols = frame_.cols;
                const int rows = frame_.rows;
                const int pitch = cols * 3;
                for (int y = 0; y < rows; y++)
                {
                    for (int x = 0; x < cols; x++)
                    {
                        unsigned char* dest = &frame.frame[y * pitch + 3 * x];
                        const cv::Vec3b& elt = frame_.at<cv::Vec3b>(y, x);
                        const cv::Scalar elt2 = static_cast<cv::Scalar>(elt);
                        dest[0] = elt2.val[0];
                        dest[1] = elt2.val[1];
                        dest[2] = elt2.val[2];
                    }
                }
                frame.channels = frame_.channels();
                frame.width = frame_.cols;
                frame.height = frame_.rows;
            }
        }
    }
}

void Tracker::data(double* data)
{
    {
        QMutexLocker l(&frame_mtx);

        if (frame.width > 0)
        {
            videoWidget->update_image(frame.frame, frame.width, frame.height);
            frame.width = 0;
        }
    }

    {
        QMutexLocker l(&ypr_mtx);

        for (int i = 0; i < 6; i++)
            data[i] = ypr[i];
    }
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

TrackerControls::TrackerControls() : tracker(nullptr)
{
	ui.setupUi(this);
    ui.cameraName->clear();
    QList<QString> names = get_camera_names();
    names.prepend("Any available");
    ui.cameraName->addItems(names);
    tie_setting(s.camera_name, ui.cameraName);
    tie_setting(s.fps, ui.cameraFPS);
    tie_setting(s.fov, ui.cameraFOV);
    tie_setting(s.resolution, ui.resolution);
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.camera_settings, SIGNAL(pressed()), this, SLOT(camera_settings()));
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

void TrackerControls::camera_settings()
{
    if (tracker)
    {
        cv::VideoCapture* cap = ht_capture(tracker->ht);
        open_camera_settings(cap, s.camera_name, &tracker->camera_mtx);
    }
    else
        open_camera_settings(nullptr, s.camera_name, nullptr);
}
