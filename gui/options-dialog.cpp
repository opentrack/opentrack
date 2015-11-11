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
#include <QDialog>

static QString kopts_to_string(const Shortcuts::key_opts& kopts)
{
    if (static_cast<QString>(kopts.guid) != "")
        return "Joystick button " + QString::number(kopts.button);
    if (static_cast<QString>(kopts.keycode) == "")
        return "None";
    return kopts.keycode;
}

OptionsDialog::OptionsDialog()
{
    ui.setupUi(this);

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

    tie_setting(s.s_main.center_method, ui.center_method);

    connect(ui.bind_center, &QPushButton::pressed, [&]() -> void { bind_key(s.center, ui.center_text); });
    connect(ui.bind_zero, &QPushButton::pressed, [&]() -> void { bind_key(s.zero, ui.zero_text); });
    connect(ui.bind_toggle, &QPushButton::pressed, [&]() -> void { bind_key(s.toggle, ui.toggle_text); });

    ui.center_text->setText(kopts_to_string(s.center));
    ui.toggle_text->setText(kopts_to_string(s.toggle));
    ui.zero_text->setText(kopts_to_string(s.zero));
}

void OptionsDialog::bind_key(Shortcuts::key_opts& kopts, QLabel* label)
{
    kopts.button = -1;
    kopts.guid = "";
    kopts.keycode = "";
    QDialog d;
    auto l = new QHBoxLayout;
    l->setMargin(0);
    auto k = new KeyboardListener;
    l->addWidget(k);
    d.setLayout(l);
    d.setFixedSize(QSize(500, 300));
    d.setWindowFlags(Qt::Dialog);
    connect(k, &KeyboardListener::key_pressed, [&] (QKeySequence s) -> void {
        kopts.keycode = s.toString(QKeySequence::PortableText);
        kopts.guid = "";
        kopts.button = -1;
        d.close();
    });
    connect(k, &KeyboardListener::joystick_button_pressed, [&](QString guid, int idx) -> void {
        kopts.guid = guid;
        kopts.keycode = "";
        kopts.button = idx;
        d.close();
    });
    d.exec();
    label->setText(kopts_to_string(kopts));
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

