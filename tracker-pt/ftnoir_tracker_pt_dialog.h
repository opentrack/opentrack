/* Copyright (c) 2012 Patrick Ruoff
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#pragma once

#include "pt-api.hpp"

#include "ftnoir_tracker_pt.h"
#include "tracker-pt/ui_FTNoIR_PT_Controls.h"
#include "cv/translation-calibrator.hpp"
#include "video/video-widget.hpp"

#include <QTimer>
#include <QMutex>

namespace pt_impl {

class TrackerDialog_PT : public ITrackerDialog
{
    Q_OBJECT
public:
    TrackerDialog_PT(const QString& module_name);
    void register_tracker(ITracker *tracker) override;
    void unregister_tracker() override;
    bool embeddable() noexcept override { return true; }
    void set_buttons_visible(bool x) override;
    void save() override;
    void reload() override;
public slots:
    void doOK();
    void doCancel();

    void startstop_trans_calib(bool start);
    void trans_calib_step();
    void poll_tracker_info_impl();
    void set_camera_settings_available(const QString& camera_name);
    void show_camera_settings();
protected:
    QString threshold_display_text(int threshold_value);

    pt_settings s;
    Tracker_PT* tracker;
    QTimer timer, calib_timer;
    TranslationCalibrator trans_calib;
    QMutex calibrator_mutex;

    Ui::UICPTClientControls ui;
};

} // ns pt_impl

using TrackerDialog_PT = pt_impl::TrackerDialog_PT;
