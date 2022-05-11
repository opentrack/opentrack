#pragma once
#include "trackhat.hpp"
#include "ui_dialog.h"
#include "tracker-pt/ftnoir_tracker_pt.h"
#include "api/plugin-api.hpp"
#include <QTimer>

class trackhat_dialog final : public ITrackerDialog
{
    Q_OBJECT

protected:
    Ui_trackhat_dialog ui;
    Tracker_PT* tracker = nullptr;
    QTimer poll_timer{this};

    pt_settings s{trackhat_metadata::module_name};
    trackhat_settings t;

    void set_buttons_visible(bool x) override;
    void update_raw_data();

public:
    trackhat_dialog();
    ~trackhat_dialog() override;
    void register_tracker(ITracker *tracker) override;
    void unregister_tracker() override;
    bool embeddable() noexcept override { return true; }
    void save() override;
    void reload() override;

public slots:
    void doOK();
    void doCancel();
    void poll_tracker_info();
};
