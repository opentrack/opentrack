/* Copyright (c) 2017 Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#undef NDEBUG

#include "fusion.h"
#include "compat/library-path.hpp"

#include <QDebug>
#include <QMessageBox>
#include <QApplication>
#include <algorithm>
#include <cassert>

static const char* own_name = "fusion";

static auto get_modules()
{
    return Modules(OPENTRACK_BASE_PATH + OPENTRACK_LIBRARY_PATH, dylib_load_quiet);
}

fusion_tracker::fusion_tracker() = default;

fusion_tracker::~fusion_tracker()
{
    // Stop high-rate poller first
    if (rot_poller)
    {
        rot_poller->stop();
        rot_poller = nullptr;
    }

    // CAVEAT order matters
    rot_tracker = nullptr;
    pos_tracker = nullptr;

    rot_dylib = nullptr;
    pos_dylib = nullptr;
}

const QString& fusion_tracker::caption()
{
    static const QString caption = tr("Fusion tracker");
    return caption;
}

module_status fusion_tracker::start_tracker(QFrame* frame)
{
    assert(!rot_tracker && !pos_tracker);
    assert(!rot_dylib && !pos_dylib);

    QString err;
    module_status status;

    const QString rot_tracker_name = s.rot_tracker_name().toString();
    const QString pos_tracker_name = s.pos_tracker_name().toString();

    assert(rot_tracker_name != own_name);
    assert(pos_tracker_name != own_name);

    if (rot_tracker_name.isEmpty() || pos_tracker_name.isEmpty())
    {
        err = tr("Trackers not selected.");
        goto end;
    }

    if (rot_tracker_name == pos_tracker_name)
    {
        err = tr("Select different trackers for rotation and position.");
        goto end;
    }

    {
        Modules libs = get_modules();

        for (auto& t : libs.trackers())
        {
            if (t->module_name == rot_tracker_name)
            {
                assert(!rot_dylib);
                rot_dylib = t;
            }

            if (t->module_name == pos_tracker_name)
            {
                assert(!pos_dylib);
                pos_dylib = t;
            }
        }
    }

    if (!rot_dylib || !pos_dylib)
        goto end;

    rot_tracker = make_dylib_instance<ITracker>(rot_dylib);
    pos_tracker = make_dylib_instance<ITracker>(pos_dylib);

    status = pos_tracker->start_tracker(frame);

    if (!status.is_ok())
    {
        err = pos_dylib->name + QStringLiteral(":\n    ") + status.error;
        goto end;
    }

    if (frame->layout() == nullptr)
    {
        status = rot_tracker->start_tracker(frame);
        if (!status.is_ok())
        {
            err = rot_dylib->name + QStringLiteral(":\n    ") + status.error;
            goto end;
        }
    }
    else
    {
        other_frame->setFixedSize(320, 240); // XXX magic frame size
        other_frame->setVisible(false);

        rot_tracker->start_tracker(&*other_frame);
    }

end:
    if (!err.isEmpty())
        return error(err);

    if (s.enable_highrate_mode && rot_tracker)
    {
        const int buffer_ms = std::max(1, static_cast<int>(s.highrate_buffer_ms));
        rot_poller = std::make_unique<highrate_poller>(rot_tracker, buffer_ms, 1);
        last_highrate_export_time = std::chrono::steady_clock::now();
        rot_poller->start();
    }

    last_highrate_export_time = std::chrono::steady_clock::now();

    return status_ok();
}

void fusion_tracker::data(double *data)
{
    if (pos_tracker && rot_tracker)
    {
        // In high-rate mode, rot_tracker is polled by separate thread
        // so we only update from pos_tracker here
        if (!s.enable_highrate_mode)
            rot_tracker->data(rot_tracker_data);
        
        pos_tracker->data(pos_tracker_data);

        for (unsigned k = 0; k < 3; k++)
            data[k] = pos_tracker_data[k];
        for (unsigned k = 3; k < 6; k++)
            data[k] = rot_tracker_data[k];
        
        // In high-rate mode, also update from latest buffered sample
        if (s.enable_highrate_mode && rot_poller)
        {
            timed_pose_sample latest_sample;
            if (rot_poller->get_latest_sample(latest_sample))
            {
                // Use most recent sample for rotation
                for (unsigned k = 3; k < 6; k++)
                {
                    data[k] = latest_sample.pose[k];
                    rot_tracker_data[k] = latest_sample.pose[k];
                }
            }
        }
    }
}

fusion_dialog::fusion_dialog()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    
    // Enable/disable buffer size based on high-rate mode checkbox
    connect(ui.enable_highrate, &QCheckBox::toggled, [this](bool checked) {
        ui.label_buffer->setEnabled(checked);
        ui.highrate_buffer_ms->setEnabled(checked);
    });

    ui.rot_tracker->addItem({});
    ui.pos_tracker->addItem({});

    Modules libs = get_modules();

    for (auto& m : libs.trackers())
    {
        if (m->module_name == own_name)
            continue;

        ui.rot_tracker->addItem(m->icon, m->name, QVariant(m->module_name));
        ui.pos_tracker->addItem(m->icon, m->name, QVariant(m->module_name));
    }

    ui.rot_tracker->setCurrentIndex(0);
    ui.pos_tracker->setCurrentIndex(0);

    tie_setting(s.rot_tracker_name, ui.rot_tracker);
    tie_setting(s.pos_tracker_name, ui.pos_tracker);
    tie_setting(s.enable_highrate_mode, ui.enable_highrate);
    tie_setting(s.highrate_buffer_ms, ui.highrate_buffer_ms);
    
    // Update UI state based on loaded settings
    ui.label_buffer->setEnabled(s.enable_highrate_mode);
    ui.highrate_buffer_ms->setEnabled(s.enable_highrate_mode);
}

void fusion_dialog::doOK()
{
    const int rot_idx = ui.rot_tracker->currentIndex() - 1;
    const int pos_idx = ui.pos_tracker->currentIndex() - 1;

    if (rot_idx == -1 || pos_idx == -1 || rot_idx == pos_idx)
    {
        QMessageBox::warning(this,
                             fusion_tracker::caption(),
                             tr("Fusion tracker only works when distinct trackers are selected "
                                "for rotation and position."),
                             QMessageBox::Close);
    }

    s.b->save();
    close();
}

void fusion_dialog::doCancel()
{
    close();
}

fusion_settings::fusion_settings() :
    opts("fusion-tracker"),
    rot_tracker_name(b, "rot-tracker", ""),
    pos_tracker_name(b, "pos-tracker", ""),
    enable_highrate_mode(b, "enable-highrate", false),
    highrate_buffer_ms(b, "highrate-buffer-ms", 50)
{
}

// High-rate poller implementation
highrate_poller::highrate_poller(std::shared_ptr<ITracker> tracker, int buffer_ms, int source_id)
    : tracker(tracker), max_buffer_size((buffer_ms * 1000) / 1), source_id(source_id)
{
    // deque doesn't have reserve(), capacity grows automatically
}

highrate_poller::~highrate_poller()
{
    stop();
}

void highrate_poller::stop()
{
    should_quit = true;
    requestInterruption();
    wait();
}

void highrate_poller::run()
{
    qDebug() << "fusion: high-rate polling thread started for source" << source_id;
    
    while (!should_quit && !isInterruptionRequested())
    {
        timed_pose_sample sample;
        sample.timestamp = std::chrono::steady_clock::now();
        sample.source_id = source_id;
        
        tracker->data(sample.pose);
        
        {
            QMutexLocker lock(&buffer_mutex);
            sample_buffer.push_back(sample);
            
            // Keep buffer size limited
            while (sample_buffer.size() > (size_t)max_buffer_size)
                sample_buffer.pop_front();
        }
        
        // Target 1000Hz (1ms sleep)
        QThread::usleep(1000);
    }
    
    qDebug() << "fusion: high-rate polling thread stopped";
}

void highrate_poller::get_samples(std::vector<timed_pose_sample>& out, std::chrono::steady_clock::time_point since)
{
    QMutexLocker lock(&buffer_mutex);
    
    for (const auto& sample : sample_buffer)
    {
        if (sample.timestamp >= since)
            out.push_back(sample);
    }
}

bool highrate_poller::get_latest_sample(timed_pose_sample& out)
{
    QMutexLocker lock(&buffer_mutex);
    if (sample_buffer.empty())
        return false;

    out = sample_buffer.back();
    return true;
}

bool fusion_tracker::get_highrate_samples(std::vector<highrate_pose_sample>& out)
{
    if (!rot_poller)
        return false;

    std::vector<timed_pose_sample> timed_samples;
    rot_poller->get_samples(timed_samples, last_highrate_export_time);
    if (timed_samples.empty())
        return false;

    auto prev_time = last_highrate_export_time;
    for (const auto& sample : timed_samples)
    {
        highrate_pose_sample converted;
        const auto dt = std::chrono::duration<double>(sample.timestamp - prev_time).count();
        converted.dt_seconds = std::clamp(dt, 1e-5, 0.1);
        std::copy(sample.pose, sample.pose + 6, converted.pose);
        converted.source_id = sample.source_id;
        out.push_back(converted);
        prev_time = sample.timestamp;
    }

    last_highrate_export_time = timed_samples.back().timestamp;
    return true;
}

OPENTRACK_DECLARE_TRACKER(fusion_tracker, fusion_dialog, fusion_metadata)
