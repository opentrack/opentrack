/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "settings.hpp"
#include "keyboard.h"
#include "compat/library-path.hpp"

#include <utility>

#include <QPushButton>
#include <QLayout>
#include <QDialog>
#include <QFileDialog>

using namespace options;
using namespace options::globals;

QString options_dialog::kopts_to_string(const key_opts& kopts)
{
    if (!kopts.guid->isEmpty())
    {
        const int btn = kopts.button & ~Qt::KeyboardModifierMask;
        const int mods = kopts.button & Qt::KeyboardModifierMask;
        QString mm;
        if (mods & Qt::ControlModifier) mm += "Control+";
        if (mods & Qt::AltModifier) mm += "Alt+";
        if (mods & Qt::ShiftModifier) mm += "Shift+";
        return mm + tr("Joy button %1").arg(QString::number(btn));
    }
    if (kopts.keycode->isEmpty())
        return tr("None");
    return kopts.keycode;
}

void options_dialog::set_disable_translation_state(bool value)
{
    with_global_settings_object([&](QSettings& s)
    {
        s.setValue("disable-translation", value);
    });
}

options_dialog::options_dialog(std::function<void(bool)>&& pause_keybindings) :
    pause_keybindings(std::move(pause_keybindings))
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(main.tray_enabled, ui.trayp);
    tie_setting(main.tray_start, ui.tray_start);

    tie_setting(main.center_at_startup, ui.center_at_startup);

    const reltrans_state reltrans_modes[] = {
        reltrans_disabled,
        reltrans_enabled,
        reltrans_non_center,
    };

    for (unsigned k = 0; k < 3; k++)
        ui.reltrans_mode->setItemData(k, int(reltrans_modes[k]));

    tie_setting(main.reltrans_mode, ui.reltrans_mode);

    tie_setting(main.reltrans_disable_tx, ui.tcomp_tx_disable);
    tie_setting(main.reltrans_disable_ty, ui.tcomp_ty_disable);
    tie_setting(main.reltrans_disable_tz, ui.tcomp_tz_disable);

    tie_setting(main.reltrans_disable_src_yaw, ui.tcomp_src_yaw_disable);
    tie_setting(main.reltrans_disable_src_pitch, ui.tcomp_src_pitch_disable);
    tie_setting(main.reltrans_disable_src_roll, ui.tcomp_src_roll_disable);

    tie_setting(main.neck_z, ui.neck_z);

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

    //tie_setting(main.center_method, ui.center_method);

    tie_setting(main.tracklogging_enabled, ui.tracklogging_enabled);

    tie_setting(main.neck_enable, ui.neck_enable);

    const bool is_translation_disabled = with_global_settings_object([] (QSettings& s) {
        return s.value("disable-translation", false).toBool();
    });
    ui.disable_translation->setChecked(is_translation_disabled);

    struct tmp
    {
        key_opts& opt;
        QLabel* label;
        QPushButton* button;
    } tuples[] =
    {
        { main.key_center1, ui.center_text, ui.bind_center },
        { main.key_center2, ui.center_text_2, ui.bind_center_2 },

        { main.key_toggle1, ui.toggle_text, ui.bind_toggle },
        { main.key_toggle2, ui.toggle_text_2, ui.bind_toggle_2 },

        { main.key_toggle_press1, ui.toggle_held_text, ui.bind_toggle_held },
        { main.key_toggle_press2, ui.toggle_held_text_2, ui.bind_toggle_held_2 },

        { main.key_zero1, ui.zero_text, ui.bind_zero },
        { main.key_zero2, ui.zero_text_2, ui.bind_zero_2 },

        { main.key_zero_press1, ui.zero_held_text, ui.bind_zero_held },
        { main.key_zero_press2, ui.zero_held_text_2, ui.bind_zero_held_2 },

        { main.key_start_tracking1, ui.start_tracking_text, ui.bind_start },
        { main.key_start_tracking2, ui.start_tracking_text_2, ui.bind_start_2 },

        { main.key_stop_tracking1, ui.stop_tracking_text , ui.bind_stop },
        { main.key_stop_tracking2, ui.stop_tracking_text_2 , ui.bind_stop_2 },

        { main.key_toggle_tracking1, ui.toggle_tracking_text, ui.bind_toggle_tracking },
        { main.key_toggle_tracking2, ui.toggle_tracking_text_2, ui.bind_toggle_tracking_2 },

        { main.key_restart_tracking1, ui.restart_tracking_text, ui.bind_restart_tracking },
        { main.key_restart_tracking2, ui.restart_tracking_text_2, ui.bind_restart_tracking_2 },
    };

    for (const tmp& val_ : tuples)
    {
        tmp val = val_;
        val.label->setText(kopts_to_string(val.opt));
        connect(&val.opt.keycode,
                static_cast<void (value_::*)(const QString&) const>(&value_::valueChanged),
                val.label,
                [=](const QString&) { val.label->setText(kopts_to_string(val.opt)); });
        {
            connect(val.button, &QPushButton::clicked, this, [=] { bind_key(val.opt, val.label); });
        }
    }
}

void options_dialog::closeEvent(QCloseEvent *)
{
    done(result());
}

void options_dialog::bind_key(key_opts& kopts, QLabel* label)
{
    kopts.button = -1;
    kopts.guid = {};
    kopts.keycode = {};
    auto k = new keyboard_listener;
    k->setWindowModality(Qt::ApplicationModal);
    k->deleteLater();

    connect(k,
            &keyboard_listener::key_pressed,
            this,
            [&](const QKeySequence& s)
            {
                kopts.keycode = s.toString(QKeySequence::PortableText);
                kopts.guid = {};
                kopts.button = -1;
                k->close();
            });
    connect(k, &keyboard_listener::joystick_button_pressed,
            this,
            [&](const QString& guid, int idx, bool held)
            {
                if (!held)
                {
                    kopts.guid = guid;
                    kopts.keycode = {};
                    kopts.button = idx;
                    k->close();
                }
            });
    connect(main.b.get(), &options::detail::bundle::reloading, k, &QDialog::close);
    pause_keybindings(true);
    k->exec();
    pause_keybindings(false);
    const bool is_crap = progn(
        for (const QChar& c : kopts.keycode())
            if (!c.isPrint())
                return true;
        return false;
    );
    if (is_crap)
    {
        kopts.keycode = {};
        kopts.guid = {};
        kopts.button = -1;
        label->setText(tr("None"));
    }
    else
        label->setText(kopts_to_string(kopts));
}

void options_dialog::doOK()
{
    if (isHidden()) // close() can return true twice in a row it seems
        return;
    hide();
    if (!close()) // dialog was closed already
        return;

    main.b->save();
    ui.game_detector->save();
    set_disable_translation_state(ui.disable_translation->isChecked());
    emit closing();
}

void options_dialog::doCancel()
{
    if (isHidden()) // close() can return true twice in a row it seems
        return;
    hide();
    if (!close()) // dialog was closed already
        return;

    main.b->reload();
    ui.game_detector->revert();
    emit closing();
}

void options_dialog::done(int res)
{
    if (isVisible())
    {
        if (res == QDialog::Accepted)
            doOK();
        else
            doCancel();
    }
}
