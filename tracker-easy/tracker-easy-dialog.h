/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "tracker-easy.h"
#include "settings.h"
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
    private:
        void UpdateCustomModelControls();

    public slots:
        void doOK();
        void doCancel();

        void set_camera_settings_available(const QString& camera_name);
        void show_camera_settings();
    signals:
        void poll_tracker_info();
    protected:

        Settings s;
        Tracker* tracker;

        Ui::UICPTClientControls ui;
    };

}
