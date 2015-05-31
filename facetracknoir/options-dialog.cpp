#include "options-dialog.hpp"

OptionsDialog::OptionsDialog()
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
    
    tie_setting(s.s_main.tcomp_p, ui.tcomp_enable);
    tie_setting(s.s_main.tcomp_tz, ui.tcomp_rz);

    tie_setting(s.s_main.a_x.zero, ui.pos_tx);
    tie_setting(s.s_main.a_y.zero, ui.pos_ty);
    tie_setting(s.s_main.a_z.zero, ui.pos_tz);
    tie_setting(s.s_main.a_yaw.zero, ui.pos_rx);
    tie_setting(s.s_main.a_pitch.zero, ui.pos_ry);
    tie_setting(s.s_main.a_roll.zero, ui.pos_rz);

    tie_setting(s.s_main.a_yaw.invert, ui.invert_yaw);
    tie_setting(s.s_main.a_pitch.invert, ui.invert_pitch);
    tie_setting(s.s_main.a_roll.invert, ui.invert_roll);
    tie_setting(s.s_main.a_x.invert, ui.invert_x);
    tie_setting(s.s_main.a_y.invert, ui.invert_y);
    tie_setting(s.s_main.a_z.invert, ui.invert_z);

    tie_setting(s.s_main.a_yaw.src, ui.src_yaw);
    tie_setting(s.s_main.a_pitch.src, ui.src_pitch);
    tie_setting(s.s_main.a_roll.src, ui.src_roll);
    tie_setting(s.s_main.a_x.src, ui.src_x);
    tie_setting(s.s_main.a_y.src, ui.src_y);
    tie_setting(s.s_main.a_z.src, ui.src_z);
    
    tie_setting(s.s_main.camera_yaw, ui.camera_yaw);
    tie_setting(s.s_main.camera_pitch, ui.camera_pitch);
    tie_setting(s.s_main.camera_roll, ui.camera_roll);
}

void OptionsDialog::doOK() {
    s.b->save();
    s.s_main.b->save();
    ui.game_detector->save();
    this->close();
    emit reload();
}

void OptionsDialog::doCancel() {
    s.b->reload();
    s.s_main.b->reload();
    ui.game_detector->revert();
    close();
}
