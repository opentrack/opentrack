#pragma once

#include "export.hpp"

#include "gui/ui_settings-dialog.h"
#include "logic/shortcuts.h"

#include <functional>

#include <QObject>
#include <QDialog>
#include <QWidget>

class OTR_GUI_EXPORT options_dialog final : public QDialog
{
    Q_OBJECT
signals:
    void closing();
public:
    options_dialog(std::unique_ptr<ITrackerDialog>& tracker_dialog, std::function<void(bool)> pause_keybindings);
    ~options_dialog() override;
    inline bool embeddable() noexcept { return false; }
    void switch_to_tracker_tab();
    void register_tracker(ITracker* t);
    void unregister_tracker();
    void tracker_module_changed();
    void save();
    void reload();
private:
    void closeEvent(QCloseEvent*) override;
    static QString kopts_to_string(const key_opts& opts);

    main_settings main;
    std::function<void(bool)> pause_keybindings;
    Ui::options_dialog ui;
    ITrackerDialog* tracker_dialog = nullptr;

private slots:
    void doOK();
    void doCancel();
    void bind_key(key_opts &kopts, QLabel* label);
    void set_disable_translation_state(bool value);
};
