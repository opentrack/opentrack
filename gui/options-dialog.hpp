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
    void reload();
public:
    OptionsDialog(std::function<void(bool)> pause_keybindings);
private:
    main_settings main;
    std::function<void(bool)> pause_keybindings;
    Ui::UI_Settings ui;
    void closeEvent(QCloseEvent *) override { doCancel(); }
private slots:
    void doOK();
    void doCancel();
    void bind_key(key_opts &kopts, QLabel* label);
};
