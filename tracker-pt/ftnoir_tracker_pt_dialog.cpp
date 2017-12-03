/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_tracker_pt_dialog.h"
#include "cv/video-property-page.hpp"

#include "compat/camera-names.hpp"
#include <opencv2/core.hpp>
#include <QString>
#include <QDebug>

TrackerDialog_PT::TrackerDialog_PT()
    : tracker(nullptr),
      timer(this),
      trans_calib(1, 2, 0)
{
    ui.setupUi(this);

    ui.camdevice_combo->addItems(get_camera_names());

    tie_setting(s.camera_name, ui.camdevice_combo);
    tie_setting(s.cam_res_x, ui.res_x_spin);
    tie_setting(s.cam_res_y, ui.res_y_spin);
    tie_setting(s.cam_fps, ui.fps_spin);

    tie_setting(s.threshold_slider, ui.threshold_slider);

    tie_setting(s.min_point_size, ui.mindiam_spin);
    tie_setting(s.max_point_size, ui.maxdiam_spin);

    tie_setting(s.clip_by, ui.clip_bheight_spin);
    tie_setting(s.clip_bz, ui.clip_blength_spin);
    tie_setting(s.clip_ty, ui.clip_theight_spin);
    tie_setting(s.clip_tz, ui.clip_tlength_spin);

    tie_setting(s.cap_x, ui.cap_width_spin);
    tie_setting(s.cap_y, ui.cap_height_spin);
    tie_setting(s.cap_z, ui.cap_length_spin);

    tie_setting(s.m01_x, ui.m1x_spin);
    tie_setting(s.m01_y, ui.m1y_spin);
    tie_setting(s.m01_z, ui.m1z_spin);

    tie_setting(s.m02_x, ui.m2x_spin);
    tie_setting(s.m02_y, ui.m2y_spin);
    tie_setting(s.m02_z, ui.m2z_spin);

    tie_setting(s.t_MH_x, ui.tx_spin);
    tie_setting(s.t_MH_y, ui.ty_spin);
    tie_setting(s.t_MH_z, ui.tz_spin);

    tie_setting(s.fov, ui.fov);

    tie_setting(s.active_model_panel, ui.model_tabs);

    tie_setting(s.dynamic_pose, ui.dynamic_pose);
    tie_setting(s.init_phase_timeout, ui.init_phase_timeout);

    tie_setting(s.auto_threshold, ui.auto_threshold);

    connect(ui.tcalib_button,SIGNAL(toggled(bool)), this, SLOT(startstop_trans_calib(bool)));

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    connect(ui.camdevice_combo, &QComboBox::currentTextChanged, this, &TrackerDialog_PT::set_camera_settings_available);
    set_camera_settings_available(ui.camdevice_combo->currentText());
    connect(ui.camera_settings, &QPushButton::clicked, this, &TrackerDialog_PT::show_camera_settings);

    connect(&timer, &QTimer::timeout, this, &TrackerDialog_PT::poll_tracker_info_impl);
    timer.setInterval(250);

    connect(&calib_timer, &QTimer::timeout, this, &TrackerDialog_PT::trans_calib_step);
    calib_timer.setInterval(35);

    poll_tracker_info_impl();

    connect(this, &TrackerDialog_PT::poll_tracker_info, this, &TrackerDialog_PT::poll_tracker_info_impl, Qt::DirectConnection);

    static constexpr pt_color_type color_types[] = {
        pt_color_average,
        pt_color_natural,
        pt_color_red_only,
        pt_color_blue_only,
    };

    for (unsigned k = 0; k < std::size(color_types); k++)
        ui.blob_color->setItemData(k, int(color_types[k]));

    tie_setting(s.blob_color, ui.blob_color);

    tie_setting(s.threshold_slider, ui.threshold_value_display, [this](const slider_value& val) {
        return threshold_display_text(int(val));
    });

    // refresh threshold display on auto-threshold checkbox state change
    tie_setting(s.auto_threshold,
                this,
                [this](bool) { s.threshold_slider.notify(); });
}

QString TrackerDialog_PT::threshold_display_text(int threshold_value)
{
    if (!s.auto_threshold)
        return tr("Brightness %1/255").arg(threshold_value);
    else
    {
        CamInfo info;
        int w = s.cam_res_x, h = s.cam_res_y;

        if (w * h <= 0)
            w = 640, h = 480;

        if (tracker && tracker->get_cam_info(&info) && info.res_x * info.res_y != 0)
            w = info.res_x, h = info.res_y;

        double value = PointExtractor::threshold_radius_value(w, h, threshold_value);

        return tr("LED radius %1 pixels").arg(value, 0, 'f', 2);
    }
}

void TrackerDialog_PT::startstop_trans_calib(bool start)
{
    QMutexLocker l(&calibrator_mutex);

    if (start)
    {
        qDebug() << "pt: starting translation calibration";
        calib_timer.start();
        trans_calib.reset();
        s.t_MH_x = 0;
        s.t_MH_y = 0;
        s.t_MH_z = 0;

        ui.sample_count_display->setText(QString());
    }
    else
    {
        calib_timer.stop();
        qDebug() << "pt: stopping translation calibration";
        {
            cv::Vec3f tmp;
            cv::Vec3i nsamples;
            std::tie(tmp, nsamples) = trans_calib.get_estimate();
            s.t_MH_x = int(tmp[0]);
            s.t_MH_y = int(tmp[1]);
            s.t_MH_z = int(tmp[2]);

            constexpr int min_yaw_samples = 15;
            constexpr int min_pitch_samples = 15;
            constexpr int min_samples = min_yaw_samples+min_pitch_samples;

            // Don't bother counting roll samples. Roll calibration is hard enough
            // that it's a hidden unsupported feature anyway.

            const QString sample_feedback = progn(
                if (nsamples[0] < min_yaw_samples)
                    return tr("%1 yaw samples. Yaw more to %2 samples for stable calibration.")
                        .arg(nsamples[0]).arg(min_yaw_samples);
                if (nsamples[1] < min_pitch_samples)
                    return tr("%1 pitch samples. Pitch more to %2 samples for stable calibration.")
                        .arg(nsamples[1]).arg(min_pitch_samples);

                const unsigned nsamples_total = nsamples[0] + nsamples[1];

                return tr("%1 samples. Over %2, good!").arg(nsamples_total).arg(min_samples);
            );

            ui.sample_count_display->setText(sample_feedback);
        }
    }
    ui.tx_spin->setEnabled(!start);
    ui.ty_spin->setEnabled(!start);
    ui.tz_spin->setEnabled(!start);
    ui.tcalib_button->setText(progn(
        if (start)
          return tr("Stop calibration");
        else
          return tr("Start calibration");
    ));
}

void TrackerDialog_PT::poll_tracker_info_impl()
{
    CamInfo info;
    if (tracker && tracker->get_cam_info(&info))
    {
        ui.caminfo_label->setText(tr("%1x%2 @ %3 FPS").arg(info.res_x).arg(info.res_y).arg(iround(info.fps)));

        // display point info
        const int n_points = tracker->get_n_points();
        ui.pointinfo_label->setText((n_points == 3 ? tr("%1 OK!") : tr("%1 BAD!")).arg(n_points));
    }
    else
    {
        ui.caminfo_label->setText(tr("Tracker offline"));
        ui.pointinfo_label->setText(QString());
    }
}

void TrackerDialog_PT::set_camera_settings_available(const QString& camera_name)
{
    const bool avail = video_property_page::should_show_dialog(camera_name);
    ui.camera_settings->setEnabled(avail);
}

void TrackerDialog_PT::show_camera_settings()
{
    const int idx = ui.camdevice_combo->currentIndex();

    if (tracker)
    {
        if (tracker->camera)
        {
            cv::VideoCapture& cap = *tracker->camera;

            CamInfo info;
            bool status;
            std::tie(status, info) = tracker->camera.get_info();
            if (status)
                video_property_page::show_from_capture(cap, info.idx);
        }
    }
    else
        video_property_page::show(idx);
}

void TrackerDialog_PT::trans_calib_step()
{
    QMutexLocker l(&calibrator_mutex);

    if (tracker)
    {
        Affine X_CM = tracker->pose();
        trans_calib.update(X_CM.R, X_CM.t);
    }
    else
        startstop_trans_calib(false);
}

void TrackerDialog_PT::save()
{
    s.b->save();
}

void TrackerDialog_PT::doOK()
{
    save();
    close();
}

void TrackerDialog_PT::doCancel()
{
    close();
}

void TrackerDialog_PT::register_tracker(ITracker *t)
{
    tracker = static_cast<Tracker_PT*>(t);
    ui.tcalib_button->setEnabled(true);
    poll_tracker_info();
    timer.start();
}

void TrackerDialog_PT::unregister_tracker()
{
    tracker = NULL;
    ui.tcalib_button->setEnabled(false);
    poll_tracker_info();
    timer.stop();
}
