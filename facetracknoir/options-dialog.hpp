#pragma once

#include <QObject>
#include <QWidget>
#include <QTimer>
#include "ui_settings.h"
#include "opentrack/state.hpp"
#include "opentrack/shortcuts.h"
#include "ftnoir_tracker_pt/ftnoir_tracker_pt_settings.h"
#include "facetracknoir/trans_calib.h"
#include "ftnoir_tracker_pt/ftnoir_tracker_pt.h"
#include "ftnoir_filter_accela/ftnoir_filter_accela.h"

class OptionsDialog: public QWidget
{
    Q_OBJECT
signals:
    void reload();
public:
    OptionsDialog(State &state);
private:
    Ui::UI_Settings ui;
    Shortcuts::settings s;
    settings_pt pt;
    settings_accela acc;
    QTimer timer;
    State& state;
    TranslationCalibrator trans_calib;
    bool trans_calib_running;

    Tracker_PT* get_pt();
    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void update_ewma_display(int value);
    void update_rot_display(int value);
    void update_trans_display(int value);
    void update_rot_dz_display(int value);
    void update_trans_dz_display(int value);

    void doOK();
    void doCancel();
    void startstop_trans_calib(bool start);
    void poll_tracker_info();
    void trans_calib_step();
    void bind_key(value<QString>& ret, QLabel* label);
};
