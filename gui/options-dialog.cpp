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

static QString kopts_to_string(const key_opts& kopts)
{
    if (static_cast<QString>(kopts.guid) != "")
    {
        const int btn = kopts.button & ~Qt::KeyboardModifierMask;
        const int mods = kopts.button & Qt::KeyboardModifierMask;
        QString mm;
        if (mods & Qt::ControlModifier) mm += "Control+";
        if (mods & Qt::AltModifier) mm += "Alt+";
        if (mods & Qt::ShiftModifier) mm += "Shift+";
        return mm + "Joy button " + QString::number(btn);
    }
    if (static_cast<QString>(kopts.keycode) == "")
        return "None";
    return kopts.keycode;
}

OptionsDialog::OptionsDialog(main_settings& main,
                             std::function<void()> register_global_keys,
                             std::function<void(bool)> pause_keybindings) :
    main(main),
    register_global_keys(register_global_keys),
    pause_keybindings(pause_keybindings)
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(main.tray_enabled, ui.trayp);
    
    tie_setting(main.center_at_startup, ui.center_at_startup);
    
    tie_setting(main.tcomp_p, ui.tcomp_enable);
    tie_setting(main.tcomp_tz, ui.tcomp_rz);

    tie_setting(main.a_x.zero, ui.pos_tx);
    tie_setting(main.a_y.zero, ui.pos_ty);
    tie_setting(main.a_z.zero, ui.pos_tz);
    tie_setting(main.a_yaw.zero, ui.pos_rx);
    tie_setting(main.a_pitch.zero, ui.pos_ry);
    tie_setting(main.a_roll.zero, ui.pos_rz);

    tie_setting(main.a_yaw.invert, ui.invert_yaw);
    tie_setting(main.a_pitch.invert, ui.invert_pitch);
    tie_setting(main.a_roll.invert, ui.invert_roll);
    tie_setting(main.a_x.invert, ui.invert_x);
    tie_setting(main.a_y.invert, ui.invert_y);
    tie_setting(main.a_z.invert, ui.invert_z);

    tie_setting(main.a_yaw.src, ui.src_yaw);
    tie_setting(main.a_pitch.src, ui.src_pitch);
    tie_setting(main.a_roll.src, ui.src_roll);
    tie_setting(main.a_x.src, ui.src_x);
    tie_setting(main.a_y.src, ui.src_y);
    tie_setting(main.a_z.src, ui.src_z);
    
    tie_setting(main.camera_yaw, ui.camera_yaw);
    tie_setting(main.camera_pitch, ui.camera_pitch);
    tie_setting(main.camera_roll, ui.camera_roll);

    tie_setting(main.center_method, ui.center_method);

    connect(ui.bind_center, &QPushButton::pressed, [&]() -> void { bind_key(main.key_center, ui.center_text); });
    connect(ui.bind_zero, &QPushButton::pressed, [&]() -> void { bind_key(main.key_zero, ui.zero_text); });
    connect(ui.bind_toggle, &QPushButton::pressed, [&]() -> void { bind_key(main.key_toggle, ui.toggle_text); });
    connect(ui.bind_start, &QPushButton::pressed, [&]() -> void { bind_key(main.key_start_tracking, ui.start_tracking_text); });
    connect(ui.bind_stop, &QPushButton::pressed, [&]() -> void { bind_key(main.key_stop_tracking, ui.stop_tracking_text); });
    connect(ui.bind_toggle_tracking, &QPushButton::pressed, [&]() -> void { bind_key(main.key_toggle_tracking, ui.toggle_tracking_text); });

    ui.center_text->setText(kopts_to_string(main.key_center));
    ui.toggle_text->setText(kopts_to_string(main.key_toggle));
    ui.zero_text->setText(kopts_to_string(main.key_zero));
    
    ui.start_tracking_text->setText(kopts_to_string(main.key_start_tracking));
    ui.stop_tracking_text->setText(kopts_to_string(main.key_stop_tracking));
    ui.toggle_tracking_text->setText(kopts_to_string(main.key_toggle_tracking));
}

void OptionsDialog::bind_key(key_opts& kopts, QLabel* label)
{
    kopts.button = -1;
    kopts.guid = "";
    kopts.keycode = "";
    QDialog d(this);
    auto l = new QHBoxLayout;
    l->setMargin(0);
    KeyboardListener k;
    l->addWidget(&k);
    d.setLayout(l);
    d.setFixedSize(QSize(500, 300));
    d.setWindowFlags(Qt::Dialog);
    d.setWindowModality(Qt::ApplicationModal);
    connect(&k, &KeyboardListener::key_pressed, [&] (QKeySequence s) -> void {
        kopts.keycode = s.toString(QKeySequence::PortableText);
        kopts.guid = "";
        kopts.button = -1;
        d.close();
    });
    connect(&k, &KeyboardListener::joystick_button_pressed, [&](QString guid, int idx, bool held) -> void {
        if (!held)
        {
            kopts.guid = guid;
            kopts.keycode = "";
            kopts.button = idx;
            d.close();
        }
    });
    pause_keybindings(true);
    d.show();
    d.exec();
    pause_keybindings(false);
    register_global_keys();
    label->setText(kopts_to_string(kopts));
    delete l;
}

void OptionsDialog::doOK() {
    main.b->save();
    ui.game_detector->save();
    this->close();
    emit reload();
}

void OptionsDialog::doCancel() {
    main.b->reload();
    ui.game_detector->revert();
    close();
}

