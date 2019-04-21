/* Copyright (c) 2012 Patrick Ruoff
 * Copyright (c) 2014-2015 Stanislaw Halik <sthalik@misaki.pl>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "tracker-easy-dialog.h"
#include "compat/math.hpp"
#include "video/camera.hpp"

#include <opencv2/core.hpp>
#include <opencv2/calib3d.hpp>

#include <QString>
#include <QtGlobal>
#include <QDebug>

using namespace options;

static void init_resources() { Q_INIT_RESOURCE(tracker_easy); }

namespace EasyTracker
{

    Dialog::Dialog() :
        s(KModuleName),
        tracker(nullptr),
        timer(this),
        trans_calib(1, 2)
    {
        init_resources();

        ui.setupUi(this);

        for (const QString& str : video::camera_names())
            ui.camdevice_combo->addItem(str);

        tie_setting(s.camera_name, ui.camdevice_combo);
        tie_setting(s.cam_res_x, ui.res_x_spin);
        tie_setting(s.cam_res_y, ui.res_y_spin);
        tie_setting(s.cam_fps, ui.fps_spin);

        tie_setting(s.threshold_slider, ui.threshold_slider);

        tie_setting(s.min_point_size, ui.mindiam_spin);
        tie_setting(s.max_point_size, ui.maxdiam_spin);
        tie_setting(s.DeadzoneRectHalfEdgeSize, ui.spinDeadzone);

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

        tie_setting(s.debug, ui.debug);

        tie_setting(s.auto_threshold, ui.auto_threshold);

        connect(ui.tcalib_button, SIGNAL(toggled(bool)), this, SLOT(startstop_trans_calib(bool)));

        connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
        connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

        connect(ui.camdevice_combo, &QComboBox::currentTextChanged, this, &Dialog::set_camera_settings_available);
        set_camera_settings_available(ui.camdevice_combo->currentText());
        connect(ui.camera_settings, &QPushButton::clicked, this, &Dialog::show_camera_settings);

        connect(&timer, &QTimer::timeout, this, &Dialog::poll_tracker_info_impl);
        timer.setInterval(250);

        connect(&calib_timer, &QTimer::timeout, this, &Dialog::trans_calib_step);
        calib_timer.setInterval(35);

        poll_tracker_info_impl();

        connect(this, &Dialog::poll_tracker_info, this, &Dialog::poll_tracker_info_impl, Qt::DirectConnection);


        for (unsigned k = 0; k < cv::SOLVEPNP_MAX_COUNT; k++)
            ui.comboBoxSolvers->setItemData(k, k);

        tie_setting(s.PnpSolver, ui.comboBoxSolvers);

        tie_setting(s.threshold_slider, ui.threshold_value_display, [this](const slider_value& val) {
            return threshold_display_text(int(val));
        });

        // refresh threshold display on auto-threshold checkbox state change
        tie_setting(s.auto_threshold,
            this,
            [this](bool) { s.threshold_slider.notify(); });
    }

    QString Dialog::threshold_display_text(int threshold_value)
    {
        if (!s.auto_threshold)
            return tr("Brightness %1/255").arg(threshold_value);
        else
        {
            int w = s.cam_res_x, h = s.cam_res_y;

            if (w * h <= 0)
            {
                w = 640;
                h = 480;
            }

            //SL: What are we suppose to do here?
            double value = 0.0f;

            return tr("LED radius %1 pixels").arg(value, 0, 'f', 2);
        }
    }

    void Dialog::startstop_trans_calib(bool start)
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
                auto[tmp, nsamples] = trans_calib.get_estimate();
                s.t_MH_x = int(tmp[0]);
                s.t_MH_y = int(tmp[1]);
                s.t_MH_z = int(tmp[2]);

                constexpr int min_yaw_samples = 15;
                constexpr int min_pitch_samples = 15;
                constexpr int min_samples = min_yaw_samples + min_pitch_samples;

                // Don't bother counting roll samples. Roll calibration is hard enough
                // that it's a hidden unsupported feature anyway.

                QString sample_feedback;
                if (nsamples[0] < min_yaw_samples)
                    sample_feedback = tr("%1 yaw samples. Yaw more to %2 samples for stable calibration.").arg(nsamples[0]).arg(min_yaw_samples);
                else if (nsamples[1] < min_pitch_samples)
                    sample_feedback = tr("%1 pitch samples. Pitch more to %2 samples for stable calibration.").arg(nsamples[1]).arg(min_pitch_samples);
                else
                {
                    const int nsamples_total = nsamples[0] + nsamples[1];
                    sample_feedback = tr("%1 samples. Over %2, good!").arg(nsamples_total).arg(min_samples);
                }

                ui.sample_count_display->setText(sample_feedback);
            }
        }
        ui.tx_spin->setEnabled(!start);
        ui.ty_spin->setEnabled(!start);
        ui.tz_spin->setEnabled(!start);

        if (start)
            ui.tcalib_button->setText(tr("Stop calibration"));
        else
            ui.tcalib_button->setText(tr("Start calibration"));
    }

    void Dialog::poll_tracker_info_impl()
    {
        //SL: sort this out
        /*
        pt_camera_info info;
        if (tracker && tracker->get_cam_info(info))
        {
            ui.caminfo_label->setText(tr("%1x%2 @ %3 FPS").arg(info.res_x).arg(info.res_y).arg(iround(info.fps)));

            // display point info
            const int n_points = tracker->get_n_points();
            ui.pointinfo_label->setText((n_points == 3 ? tr("%1 OK!") : tr("%1 BAD!")).arg(n_points));
        }
        else
        */
        {
            ui.caminfo_label->setText(tr("Tracker offline"));
            ui.pointinfo_label->setText(QString());
        }
    }

    void Dialog::set_camera_settings_available(const QString& /* camera_name */)
    {
        ui.camera_settings->setEnabled(true);
    }

    void Dialog::show_camera_settings()
    {
        if (tracker)
        {
            QMutexLocker l(&tracker->camera_mtx);
            tracker->camera->show_dialog();
        }
        else
            (void)video::show_dialog(s.camera_name);
    }

    void Dialog::trans_calib_step()
    {
        QMutexLocker l(&calibrator_mutex);
        // TODO: Do we still need that function
    }

    void Dialog::save()
    {
        s.b->save();
    }

    void Dialog::doOK()
    {
        save();
        close();
    }

    void Dialog::doCancel()
    {
        close();
    }

    void Dialog::register_tracker(ITracker *t)
    {
        tracker = static_cast<Tracker*>(t);
        ui.tcalib_button->setEnabled(true);
        poll_tracker_info();
        timer.start();
    }

    void Dialog::unregister_tracker()
    {
        tracker = nullptr;
        ui.tcalib_button->setEnabled(false);
        poll_tracker_info();
        timer.stop();
    }
}
