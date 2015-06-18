#include "options-dialog.hpp"
#include "ftnoir_tracker_pt/camera.h"

OptionsDialog::OptionsDialog(State& state) : state(state), trans_calib_running(false)
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
    
    tie_setting(pt.camera_mode, ui.camera_mode);

    tie_setting(pt.threshold, ui.threshold_slider);

    tie_setting(pt.min_point_size, ui.mindiam_spin);
    tie_setting(pt.max_point_size, ui.maxdiam_spin);

    tie_setting(pt.t_MH_x, ui.tx_spin);
    tie_setting(pt.t_MH_y, ui.ty_spin);
    tie_setting(pt.t_MH_z, ui.tz_spin);
    
    tie_setting(pt.fov, ui.camera_fov);
    
    tie_setting(pt.is_cap, ui.model_cap);
    
    tie_setting(acc.rot_threshold, ui.rotation_slider);
    tie_setting(acc.trans_threshold, ui.translation_slider);
    tie_setting(acc.ewma, ui.ewma_slider);
    tie_setting(acc.rot_deadzone, ui.rot_dz_slider);
    tie_setting(acc.trans_deadzone, ui.trans_dz_slider);
    
    connect(&timer,SIGNAL(timeout()), this,SLOT(poll_tracker_info()));
    connect( ui.tcalib_button,SIGNAL(toggled(bool)), this,SLOT(startstop_trans_calib(bool)) );
    timer.start(100);
}

void OptionsDialog::doOK() {
    s.b->save();
    pt.b->save();
    s.s_main.b->save();
    ui.game_detector->save();
    this->close();
    emit reload();
}

void OptionsDialog::doCancel() {
    s.b->reload();
    pt.b->reload();
    s.s_main.b->reload();
    ui.game_detector->revert();
    close();
}

void OptionsDialog::startstop_trans_calib(bool start)
{
    auto tracker = get_pt();
    if (!tracker)
    {
        ui.tcalib_button->setChecked(false);
        return;
    }
        
    if (start)
    {
        qDebug()<<"TrackerDialog:: Starting translation calibration";
        trans_calib.reset();
        trans_calib_running = true;
        pt.t_MH_x = 0;
        pt.t_MH_y = 0;
        pt.t_MH_z = 0;
    }
    else
    {
        qDebug()<<"TrackerDialog:: Stopping translation calibration";
        trans_calib_running = false;
        {
            auto tmp = trans_calib.get_estimate();
            pt.t_MH_x = tmp[0];
            pt.t_MH_y = tmp[1];
            pt.t_MH_z = tmp[2];
        }
    }
}

void OptionsDialog::poll_tracker_info()
{
    auto tracker = get_pt();
    if (tracker)
    {
        QString to_print;

        // display caminfo
        CamInfo info;
        tracker->get_cam_info(&info);
        to_print = QString::number(info.res_x)+"x"+QString::number(info.res_y)+" @ "+QString::number(info.fps)+" FPS";
        ui.caminfo_label->setText(to_print);

        // display pointinfo
        int n_points = tracker->get_n_points();
        to_print = QString::number(n_points);
        if (n_points == 3)
            to_print += " OK!";
        else
            to_print += " BAD!";
        ui.pointinfo_label->setText(to_print);

        // update calibration
        if (trans_calib_running) trans_calib_step();
    }
    else
    {
        QString to_print = "Tracker offline";
        ui.caminfo_label->setText(to_print);
        ui.pointinfo_label->setText(to_print);
    }
}

void OptionsDialog::trans_calib_step()
{
    auto tracker = get_pt();
    if (tracker)
    {
        Affine X_CM = tracker->pose();
        trans_calib.update(X_CM.R, X_CM.t);
    }
}

Tracker_PT* OptionsDialog::get_pt()
{
    auto work = state.work.get();
    if (!work)
        return nullptr;
    auto ptr = work->libs.pTracker;
    if (ptr)
        return static_cast<Tracker_PT*>(ptr.get());
    return nullptr;
}

void OptionsDialog::update_rot_display(int value)
{
    ui.rot_gain->setText(QString::number((value + 1) * 10 / 100.) + "°");
}

void OptionsDialog::update_trans_display(int value)
{
    ui.trans_gain->setText(QString::number((value + 1) * 5 / 100.) + "mm");
}

void OptionsDialog::update_ewma_display(int value)
{
    ui.ewma_label->setText(QString::number(value * 2) + "ms");
}

void OptionsDialog::update_rot_dz_display(int value)
{
    ui.rot_dz->setText(QString::number(value * 2 / 100.) + "°");
}

void OptionsDialog::update_trans_dz_display(int value)
{
    ui.trans_dz->setText(QString::number(value * 1 / 100.) + "mm");
}
