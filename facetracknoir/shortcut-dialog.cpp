#include "shortcut-dialog.hpp"

KeyboardShortcutDialog::KeyboardShortcutDialog()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    for ( int i = 0; i < global_key_sequences.size(); i++) {
        ui.cbxCenterKey->addItem(global_key_sequences.at(i));
        ui.cbxToggleKey->addItem(global_key_sequences.at(i));
        ui.cbxZeroKey->addItem(global_key_sequences.at(i));
    }

    tie_setting(s.center.key_index, ui.cbxCenterKey);
    tie_setting(s.center.alt, ui.chkCenterAlt);
    tie_setting(s.center.shift, ui.chkCenterShift);
    tie_setting(s.center.ctrl, ui.chkCenterCtrl);

    tie_setting(s.toggle.key_index, ui.cbxToggleKey);
    tie_setting(s.toggle.alt, ui.chkToggleAlt);
    tie_setting(s.toggle.shift, ui.chkToggleShift);
    tie_setting(s.toggle.ctrl, ui.chkToggleCtrl);

    tie_setting(s.zero.key_index, ui.cbxZeroKey);
    tie_setting(s.zero.alt, ui.chkZeroAlt);
    tie_setting(s.zero.shift, ui.chkZeroShift);
    tie_setting(s.zero.ctrl, ui.chkZeroCtrl);

    tie_setting(s.s_main.tray_enabled, ui.trayp);
    
    tie_setting(s.s_main.center_at_startup, ui.center_at_startup);
}

void KeyboardShortcutDialog::doOK() {
    s.b->save();
    s.s_main.b->save();
    ui.game_detector->save();
    this->close();
    emit reload();
}

void KeyboardShortcutDialog::doCancel() {
    s.b->reload();
    s.s_main.b->reload();
    ui.game_detector->revert();
    close();
}
