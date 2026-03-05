#pragma once

#include "api/plugin-api.hpp"
#include "api/plugin-support.hpp"
#include "options/options.hpp"
using namespace options;

#include <memory>
#include <deque>
#include <atomic>
#include <chrono>
#include <vector>

#include <QObject>
#include <QFrame>
#include <QThread>
#include <QMutex>
#include <QCoreApplication>

struct fusion_settings final : opts
{
    value<QVariant> rot_tracker_name, pos_tracker_name;
    value<bool> enable_highrate_mode;
    value<int> highrate_buffer_ms;

    fusion_settings();
};

// Timestamped pose sample for high-rate data
struct timed_pose_sample {
    std::chrono::steady_clock::time_point timestamp;
    double pose[6];
    int source_id; // 0=pos_tracker, 1=rot_tracker
};

// High-rate polling thread for IMU tracker
class highrate_poller : public QThread
{
    Q_OBJECT
    
    std::shared_ptr<ITracker> tracker;
    std::deque<timed_pose_sample> sample_buffer;
    QMutex buffer_mutex;
    int max_buffer_size;
    std::atomic<bool> should_quit{false};
    int source_id;

protected:
    void run() override;

public:
    highrate_poller(std::shared_ptr<ITracker> tracker, int buffer_ms, int source_id);
    ~highrate_poller() override;
    
    void get_samples(std::vector<timed_pose_sample>& out, std::chrono::steady_clock::time_point since);
    bool get_latest_sample(timed_pose_sample& out);
    void stop();
};

class fusion_tracker : public QObject, public ITracker, public IHighrateSource
{
    Q_OBJECT

    double rot_tracker_data[6] {}, pos_tracker_data[6] {};

    std::unique_ptr<QFrame> other_frame { std::make_unique<QFrame>() };
    std::shared_ptr<dylib> rot_dylib, pos_dylib;
    std::shared_ptr<ITracker> rot_tracker, pos_tracker;
    
    // High-rate mode support
    std::unique_ptr<highrate_poller> rot_poller;
    std::chrono::steady_clock::time_point last_highrate_export_time;
    fusion_settings s;

public:
    fusion_tracker();
    ~fusion_tracker() override;
    module_status start_tracker(QFrame*) override;
    void data(double* data) override;
    bool get_highrate_samples(std::vector<highrate_pose_sample>& out) override;
    
    // Extended interface for high-rate data access
    bool has_highrate_data() const { return rot_poller != nullptr; }

    static const QString& caption();
};

#include "ui_fusion.h"

class fusion_dialog : public ITrackerDialog
{
    Q_OBJECT

    fusion_settings s;
    Ui::fusion_ui ui;
public:
    fusion_dialog();
private slots:
    void doOK();
    void doCancel();
};

class fusion_metadata : public Metadata
{
    Q_OBJECT

    QString name() { return tr("Fusion"); }
    QIcon icon() { return QIcon(":/images/fusion-tracker-logo.png"); }
};

