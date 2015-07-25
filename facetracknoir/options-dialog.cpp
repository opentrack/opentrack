/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "options-dialog.hpp"
#include "keyboard.h"
#include <QPushButton>
#include <QLayout>

OptionsDialog::OptionsDialog()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

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

    connect(ui.bind_center, &QPushButton::pressed, [&]() -> void { bind_key(s.center.keycode, ui.center_text); });
    connect(ui.bind_zero, &QPushButton::pressed, [&]() -> void { bind_key(s.zero.keycode, ui.zero_text); });
    connect(ui.bind_toggle, &QPushButton::pressed, [&]() -> void { bind_key(s.toggle.keycode, ui.toggle_text); });

    ui.center_text->setText(s.center.keycode == "" ? "None" : static_cast<QString>(s.center.keycode));
    ui.toggle_text->setText(s.toggle.keycode == "" ? "None" : static_cast<QString>(s.toggle.keycode));
    ui.zero_text->setText(s.zero.keycode == "" ? "None" : static_cast<QString>(s.zero.keycode));
}

void OptionsDialog::bind_key(value<QString>& ret, QLabel* label)
{
    ret = "";
    QDialog d;
    auto l = new QHBoxLayout;
    l->setMargin(0);
    auto k = new KeyboardListener;
    l->addWidget(k);
    d.setLayout(l);
    d.setFixedSize(QSize(500, 500));
    d.setWindowFlags(Qt::Dialog);
    connect(k, &KeyboardListener::key_pressed, [&] (QKeySequence s) -> void { ret = s.toString(QKeySequence::PortableText); d.close(); });
    d.exec();
    label->setText(ret == "" ? "None" : static_cast<QString>(ret));
    delete k;
    delete l;
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
