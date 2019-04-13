/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "tracker-easy-api.h"

#include "tracker-easy.h"
#include "ui_tracker-easy-settings.h"
#include "cv/translation-calibrator.hpp"
#include "video/video-widget.hpp"

#include <QTimer>
#include <QMutex>

namespace EasyTracker
{
    class Dialog : public ITrackerDialog
    {
        Q_OBJECT
    public:
        Dialog();
        void register_tracker(ITracker *tracker) override;
        void unregister_tracker() override;
        void save();
    public slots:
        void doOK();
        void doCancel();

        void startstop_trans_calib(bool start);
        void trans_calib_step();
        void poll_tracker_info_impl();
        void set_camera_settings_available(const QString& camera_name);
        void show_camera_settings();
    signals:
        void poll_tracker_info();
    protected:
        QString threshold_display_text(int threshold_value);

        pt_settings s;
        Tracker* tracker;
        QTimer timer, calib_timer;
        TranslationCalibrator trans_calib;
        QMutex calibrator_mutex;

        Ui::UICPTClientControls ui;
    };

}
