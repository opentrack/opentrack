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

        tie_setting(s.iMinBlobSize, ui.mindiam_spin);
        tie_setting(s.iMaxBlobSize, ui.maxdiam_spin);
        tie_setting(s.DeadzoneRectHalfEdgeSize, ui.spinDeadzone);

        tie_setting(s.clip_by, ui.clip_bheight_spin);
        tie_setting(s.clip_bz, ui.clip_blength_spin);
        tie_setting(s.clip_ty, ui.clip_theight_spin);
        tie_setting(s.clip_tz, ui.clip_tlength_spin);

        tie_setting(s.cap_x, ui.cap_width_spin);
        tie_setting(s.cap_y, ui.cap_height_spin);
        tie_setting(s.cap_z, ui.cap_length_spin);

        tie_setting(s.iVertexTopX, ui.iSpinVertexTopX);
        tie_setting(s.iVertexTopY, ui.iSpinVertexTopY);
        tie_setting(s.iVertexTopZ, ui.iSpinVertexTopZ);

        tie_setting(s.iVertexRightX, ui.iSpinVertexRightX);
        tie_setting(s.iVertexRightY, ui.iSpinVertexRightY);
        tie_setting(s.iVertexRightZ, ui.iSpinVertexRightZ);

        tie_setting(s.iVertexLeftX, ui.iSpinVertexLeftX);
        tie_setting(s.iVertexLeftY, ui.iSpinVertexLeftY);
        tie_setting(s.iVertexLeftZ, ui.iSpinVertexLeftZ);

        tie_setting(s.iVertexCenterX, ui.iSpinVertexCenterX);
        tie_setting(s.iVertexCenterY, ui.iSpinVertexCenterY);
        tie_setting(s.iVertexCenterZ, ui.iSpinVertexCenterZ);

        tie_setting(s.iVertexTopRightX, ui.iSpinVertexTopRightX);
        tie_setting(s.iVertexTopRightY, ui.iSpinVertexTopRightY);
        tie_setting(s.iVertexTopRightZ, ui.iSpinVertexTopRightZ);

        tie_setting(s.iVertexTopLeftX, ui.iSpinVertexTopLeftX);
        tie_setting(s.iVertexTopLeftY, ui.iSpinVertexTopLeftY);
        tie_setting(s.iVertexTopLeftZ, ui.iSpinVertexTopLeftZ);


        tie_setting(s.fov, ui.fov);

        tie_setting(s.active_model_panel, ui.model_tabs);

        tie_setting(s.debug, ui.debug);


        connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
        connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

        connect(ui.camdevice_combo, &QComboBox::currentTextChanged, this, &Dialog::set_camera_settings_available);
        set_camera_settings_available(ui.camdevice_combo->currentText());
        connect(ui.camera_settings, &QPushButton::clicked, this, &Dialog::show_camera_settings);

        // Radio Button
        connect(ui.iRadioButtonCustomModelThree, &QRadioButton::clicked, this, &Dialog::UpdateCustomModelControls);
        connect(ui.iRadioButtonCustomModelFour, &QRadioButton::clicked, this, &Dialog::UpdateCustomModelControls);
        connect(ui.iRadioButtonCustomModelFive, &QRadioButton::clicked, this, &Dialog::UpdateCustomModelControls);

        tie_setting(s.iCustomModelThree, ui.iRadioButtonCustomModelThree);
        tie_setting(s.iCustomModelFour, ui.iRadioButtonCustomModelFour);
        tie_setting(s.iCustomModelFive, ui.iRadioButtonCustomModelFive);

        connect(&timer, &QTimer::timeout, this, &Dialog::poll_tracker_info_impl);
        timer.setInterval(250);


        poll_tracker_info_impl();

        connect(this, &Dialog::poll_tracker_info, this, &Dialog::poll_tracker_info_impl, Qt::DirectConnection);


        for (unsigned k = 0; k < cv::SOLVEPNP_MAX_COUNT; k++)
            ui.comboBoxSolvers->setItemData(k, k);

        tie_setting(s.PnpSolver, ui.comboBoxSolvers);

        UpdateCustomModelControls();
    }

    void Dialog::UpdateCustomModelControls()
    {
        if (ui.iRadioButtonCustomModelThree->isChecked())
        {
            ui.iGroupBoxCenter->hide();
            ui.iGroupBoxTopRight->hide();
            ui.iGroupBoxTopLeft->hide();
        }
        else if (ui.iRadioButtonCustomModelFour->isChecked())
        {
            ui.iGroupBoxCenter->show();
            ui.iGroupBoxTopRight->hide();
            ui.iGroupBoxTopLeft->hide();
        }
        else if (ui.iRadioButtonCustomModelFive->isChecked())
        {
            ui.iGroupBoxCenter->hide();
            ui.iGroupBoxTopRight->show();
            ui.iGroupBoxTopLeft->show();
        }

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
        poll_tracker_info();
        timer.start();
    }

    void Dialog::unregister_tracker()
    {
        tracker = nullptr;
        poll_tracker_info();
        timer.stop();
    }
}
