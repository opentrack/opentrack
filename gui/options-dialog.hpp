#pragma once

#include "ui_settings.h"
#include "opentrack-logic/shortcuts.h"
#include <QObject>
#include <QWidget>
#include <functional>

class OptionsDialog: public QWidget
{
    Q_OBJECT
signals:
    void saving();
public:
    OptionsDialog(std::function<void(bool)> pause_keybindings);
public slots:
    void update_widgets_states(bool tracker_is_running);
private:
    main_settings main;
    std::function<void(bool)> pause_keybindings;
    Ui::UI_Settings ui;
    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void doOK();
    void doCancel();
    void bind_key(key_opts &kopts, QLabel* label);
    void browse_datalogging_file();
};
