#pragma once

#include "export.hpp"

#include "gui/ui_settings-dialog.h"
#include "logic/shortcuts.h"

#include <functional>

#include <QObject>
#include <QDialog>
#include <QWidget>

class OTR_GUI_EXPORT options_dialog : public QDialog
{
    Q_OBJECT
signals:
    void closing();
public:
    explicit options_dialog(std::function<void(bool)>&& pause_keybindings);
private:
    main_settings main;
    std::function<void(bool)> pause_keybindings;
    Ui::options_dialog ui;
    void closeEvent(QCloseEvent*) override;
    static QString kopts_to_string(const key_opts& opts);
private slots:
    void doOK();
    void doCancel();
    void done(int res) override;
    void bind_key(key_opts &kopts, QLabel* label);
    void set_disable_translation_state(bool value);
};
