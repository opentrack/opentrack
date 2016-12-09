#pragma once

#include "tobii-settings.hpp"

#include "api/plugin-api.hpp"
#include "ui_tobii-eyex-dialog.h"
#include <QObject>

class tobii_eyex_dialog final : public ITrackerDialog
{
    Q_OBJECT
public:
    tobii_eyex_dialog();
    void register_tracker(ITracker*) override {}
    void unregister_tracker() override {}
private:
    Ui::tobii_eyex_dialog_widgets ui;
    settings s;
    rel_settings rs;
private slots:
    void do_ok();
    void do_cancel();
};
