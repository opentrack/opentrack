/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "options-dialog.hpp"
#include "keyboard.h"
#include "opentrack-library-path.h"
#include <QPushButton>
#include <QLayout>
#include <QDialog>
#include <QFileDialog>

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

OptionsDialog::OptionsDialog(std::function<void(bool)> pause_keybindings) :
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
    tie_setting(main.use_camera_offset_from_centering, ui.use_center_as_translation_camera_offset);

    tie_setting(main.center_method, ui.center_method);

    tie_setting(main.tracklogging_enabled, ui.tracklogging_enabled);
    tie_setting(main.tracklogging_filename, ui.tracklogging_filenameedit);

    struct tmp
    {
        key_opts& opt;
        QLabel* label;
        QPushButton* button;
    } tuples[] =
    {
        { main.key_center, ui.center_text, ui.bind_center },
        { main.key_toggle, ui.toggle_text, ui.bind_toggle },
        { main.key_toggle_press, ui.toggle_held_text, ui.bind_toggle_held },
        { main.key_zero, ui.zero_text, ui.bind_zero },
        { main.key_zero_press, ui.zero_held_text, ui.bind_zero_held },
        { main.key_start_tracking, ui.start_tracking_text, ui.bind_start },
        { main.key_stop_tracking, ui.stop_tracking_text , ui.bind_stop},
        { main.key_toggle_tracking, ui.toggle_tracking_text, ui.bind_toggle_tracking },
        { main.key_restart_tracking, ui.restart_tracking_text, ui.bind_restart_tracking }
    };

    for (const tmp& val_ : tuples)
    {
        tmp val = val_;
        val.label->setText(kopts_to_string(val.opt));
        connect(&val.opt.keycode,
                static_cast<void (base_value::*)(const QString&)>(&base_value::valueChanged),
                val.label,
                [=](const QString&) -> void { val.label->setText(kopts_to_string(val.opt)); });
        {
            connect(val.button, &QPushButton::clicked, this, [=]() -> void { bind_key(val.opt, val.label); });
        }
    }

    connect(ui.tracklogging_fileselectbtn, SIGNAL(clicked()), this, SLOT(browse_datalogging_file()));
}

void OptionsDialog::bind_key(key_opts& kopts, QLabel* label)
{
    kopts.button = -1;
    kopts.guid = "";
    kopts.keycode = "";
    QDialog d;
    QHBoxLayout l;
    l.setMargin(0);
    KeyboardListener k;
    l.addWidget(&k);
    d.setLayout(&l);
    d.setFixedSize(QSize(500, 300));
    d.setWindowFlags(Qt::Dialog);
    d.setWindowModality(Qt::ApplicationModal);
    connect(&k,
            &KeyboardListener::key_pressed,
            &d,
            [&](QKeySequence s) -> void
            {
                kopts.keycode = s.toString(QKeySequence::PortableText);
                kopts.guid = "";
                kopts.button = -1;
                d.close();
            });
    connect(&k, &KeyboardListener::joystick_button_pressed,
            &d,
            [&](QString guid, int idx, bool held) -> void
            {
                if (!held)
                {
                    kopts.guid = guid;
                    kopts.keycode = "";
                    kopts.button = idx;
                    d.close();
                }
            });
    connect(main.b.get(), &options::detail::opt_bundle::reloading, &d, &QDialog::close);
    pause_keybindings(true);
    d.show();
    d.exec();
    pause_keybindings(false);
    label->setText(kopts_to_string(kopts));
}

void OptionsDialog::doOK()
{
    main.b->save();
    ui.game_detector->save();
    close();
    emit saving();
}

void OptionsDialog::doCancel()
{
    main.b->reload();
    ui.game_detector->revert();
    close();
}

void OptionsDialog::browse_datalogging_file()
{
    QString filename = ui.tracklogging_filenameedit->text();
    if (filename.isEmpty())
        filename = OPENTRACK_BASE_PATH;
    /* Sometimes this function freezes the app before opening the dialog.
       Might be related to https://forum.qt.io/topic/49209/qfiledialog-getopenfilename-hangs-in-windows-when-using-the-native-dialog/8
       and be a known problem. Possible solution is to use the QFileDialog::DontUseNativeDialog flag.
       Since the freeze is apparently random, I'm not sure it helped.
    */
    QString newfilename = QFileDialog::getSaveFileName(this, tr("Select Filename"), filename, tr("CSV File (*.csv)"), nullptr, QFileDialog::DontUseNativeDialog);
    if (!newfilename.isEmpty())
        ui.tracklogging_filenameedit->setText(newfilename);

    // dialog likes to mess with current directory
    QDir::setCurrent(OPENTRACK_BASE_PATH);
}

void OptionsDialog::update_widgets_states(bool tracker_is_running)
{
    ui.tracklogging_enabled->setEnabled(!tracker_is_running);
    ui.tracklogging_fileselectbtn->setEnabled(!tracker_is_running);
}
