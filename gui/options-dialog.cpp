/* Copyright (c) 2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "options-dialog.hpp"
#include "listener.h"

#include <utility>

#include <QPushButton>
#include <QLayout>
#include <QDialog>
#include <QFileDialog>

using namespace options;
using namespace options::globals;

QString options_dialog::kopts_to_string(const key_opts& kopts)
{
    using namespace Qt::Literals::StringLiterals;
    if (!kopts.guid->isEmpty())
    {
        const int btn = kopts.button & ~Qt::KeyboardModifierMask;
        const int mods = kopts.button & Qt::KeyboardModifierMask;
        QString mm;
        if (mods & Qt::ControlModifier) mm += "Control+";
        if (mods & Qt::AltModifier) mm += "Alt+";
        if (mods & Qt::ShiftModifier) mm += "Shift+";
        const auto& str = kopts.guid == "mouse"_L1
                          ? tr("Mouse %1")
                          : kopts.guid->startsWith("GI!"_L1)
                          ? tr("Gamepad button %1")
                          : tr("Joy button %1");
        return mm + str.arg(QString::number(btn));
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
        mark_global_ini_modified();
    });
}

options_dialog::options_dialog(std::unique_ptr<ITrackerDialog>& tracker_dialog_,
                               std::unique_ptr<IProtocolDialog>& proto_dialog_,
                               std::unique_ptr<IFilterDialog>& filter_dialog_,
                               std::function<void(bool)> pause_keybindings) :
    pause_keybindings(std::move(pause_keybindings))
{
    ui.setupUi(this);

    tie_setting(main.tray_enabled, ui.trayp);
    tie_setting(main.tray_start, ui.tray_start);

    tie_setting(main.center_at_startup, ui.center_at_startup);

    const centering_state centering_modes[] = {
        center_disabled,
        center_point,
        center_vr360,
        center_roll_compensated,
    };
    for (int k = 0; k < 4; k++)
        ui.cbox_centering->setItemData(k, centering_modes[k]);
    tie_setting(main.centering_mode, ui.cbox_centering);

    const reltrans_state reltrans_modes[] = {
        reltrans_disabled,
        reltrans_enabled,
        reltrans_non_center,
    };

    for (int k = 0; k < 3; k++)
        ui.reltrans_mode->setItemData(k, reltrans_modes[k]);

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

    tie_setting(main.a_yaw.invert_pre, ui.invert_yaw_pre);
    tie_setting(main.a_pitch.invert_pre, ui.invert_pitch_pre);
    tie_setting(main.a_roll.invert_pre, ui.invert_roll_pre);
    tie_setting(main.a_x.invert_pre, ui.invert_x_pre);
    tie_setting(main.a_y.invert_pre, ui.invert_y_pre);
    tie_setting(main.a_z.invert_pre, ui.invert_z_pre);

    tie_setting(main.a_yaw.invert_post, ui.invert_yaw_post);
    tie_setting(main.a_pitch.invert_post, ui.invert_pitch_post);
    tie_setting(main.a_roll.invert_post, ui.invert_roll_post);
    tie_setting(main.a_x.invert_post, ui.invert_x_post);
    tie_setting(main.a_y.invert_post, ui.invert_y_post);
    tie_setting(main.a_z.invert_post, ui.invert_z_post);

    tie_setting(main.a_yaw.src, ui.src_yaw);
    tie_setting(main.a_pitch.src, ui.src_pitch);
    tie_setting(main.a_roll.src, ui.src_roll);
    tie_setting(main.a_x.src, ui.src_x);
    tie_setting(main.a_y.src, ui.src_y);
    tie_setting(main.a_z.src, ui.src_z);

    tie_setting(main.enable_camera_offset, ui.enable_camera_offset);
    tie_setting(main.camera_offset_yaw,   ui.camera_offset_yaw);
    tie_setting(main.camera_offset_pitch, ui.camera_offset_pitch);
    tie_setting(main.camera_offset_roll,  ui.camera_offset_roll);

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
            connect(val.button, &QPushButton::clicked, this, [=, this] { bind_key(val.opt, val.label); });
        }
    }

    auto add_module_tab = [this] (auto& place, auto&& dlg, const QString& label) {
        if (dlg && dlg->embeddable())
        {
            using BaseDialog = plugin_api::detail::BaseDialog;

            dlg->set_buttons_visible(false);
            place = dlg.release();
            ui.tabWidget->addTab(place, label);
            QObject::connect(place, &BaseDialog::closing, this, &QDialog::close);
        }
    };

    add_module_tab(tracker_dialog, tracker_dialog_, tr("Tracker"));
    add_module_tab(proto_dialog, proto_dialog_, tr("Output"));
    add_module_tab(filter_dialog, filter_dialog_, tr("Filter"));

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &options_dialog::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &options_dialog::reject);
    connect(this, &options_dialog::accepted, this, &options_dialog::doAccept);
    connect(this, &options_dialog::rejected, this, &options_dialog::doReject);
}

void options_dialog::bind_key(key_opts& kopts, QLabel* label)
{
    kopts.button = -1;
    kopts.guid = {};
    kopts.keycode = {};

    auto* k = new keyboard_listener;
    k->deleteLater();
    k->setWindowModality(Qt::ApplicationModal);

    {
        QObject obj;
        connect(&*k, &keyboard_listener::key_pressed,
                &obj,
                [&](const QKeySequence& s)
                {
                    kopts.keycode = s.toString(QKeySequence::PortableText);
                    kopts.guid = {};
                    kopts.button = -1;
                    k->close();
                });
        connect(&*k, &keyboard_listener::joystick_button_pressed,
                &obj,
                [&](const QString& guid, int idx, bool held)
                {
                    if (!k)
                        std::abort();
                    if (!held)
                    {
                        kopts.guid = guid;
                        kopts.keycode = {};
                        kopts.button = idx;
                        k->close();
                    }
                });
        connect(&*main.b, &options::detail::bundle::reloading, &*k, &QDialog::close);
        pause_keybindings(false);
        k->exec();
        k = nullptr;
        pause_keybindings(true);
    }
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
    //qDebug() << "bind_key done" << kopts.guid << kopts.button << kopts.keycode;
}

void options_dialog::switch_to_tracker_tab()
{
    if (tracker_dialog)
        ui.tabWidget->setCurrentWidget(tracker_dialog);
    else
        eval_once(qDebug() << "options: asked for tracker tab widget with old-style widget dialog!");
}

void options_dialog::switch_to_proto_tab()
{
    if (proto_dialog)
        ui.tabWidget->setCurrentWidget(proto_dialog);
    else
        eval_once(qDebug() << "options: asked for proto tab widget with old-style widget dialog!");
}

void options_dialog::switch_to_filter_tab()
{
    if (filter_dialog)
        ui.tabWidget->setCurrentWidget(filter_dialog);
    else
        eval_once(qDebug() << "options: asked for filter tab widget with old-style widget dialog!");
}

void options_dialog::tracker_module_changed()
{
    if (tracker_dialog)
    {
        unregister_tracker();
        reload();
        delete tracker_dialog;
        tracker_dialog = nullptr;
    }
}

void options_dialog::proto_module_changed()
{
    if (proto_dialog)
    {
        unregister_protocol();
        reload();
        delete proto_dialog;
        proto_dialog = nullptr;
    }
}

void options_dialog::filter_module_changed()
{
    if (filter_dialog)
    {
        unregister_filter();
        reload();
        delete filter_dialog;
        filter_dialog = nullptr;
    }
}

void options_dialog::register_tracker(ITracker* t)
{
    if (tracker_dialog)
        tracker_dialog->register_tracker(t);
}

void options_dialog::unregister_tracker()
{
    if (tracker_dialog)
        tracker_dialog->unregister_tracker();
}

void options_dialog::register_protocol(IProtocol* p)
{
    if (proto_dialog)
        proto_dialog->register_protocol(p);
}

void options_dialog::unregister_protocol()
{
    if (proto_dialog)
        proto_dialog->unregister_protocol();
}

void options_dialog::register_filter(IFilter* f)
{
    if (filter_dialog)
        filter_dialog->register_filter(f);
}

void options_dialog::unregister_filter()
{
    if (filter_dialog)
        filter_dialog->unregister_filter();
}

using module_list = std::initializer_list<plugin_api::detail::BaseDialog*>;

void options_dialog::save()
{
    qDebug() << "options_dialog: save";
    main.b->save();
    ui.game_detector->save();
    set_disable_translation_state(ui.disable_translation->isChecked());

    for (auto* dlg : module_list{ tracker_dialog, proto_dialog, filter_dialog })
        if (dlg)
            dlg->save();
}

void options_dialog::reload()
{
    qDebug() << "options_dialog: reload";
    ui.game_detector->revert();

    main.b->reload();
    for (auto* dlg : module_list{ tracker_dialog, proto_dialog, filter_dialog })
        if (dlg)
            dlg->reload();
}

void options_dialog::closeEvent(QCloseEvent *)
{
    qDebug() << "options_dialog: closeEvent";
    reject();
    emit closing();
}

void options_dialog::doOK()
{
    qDebug() << "options_dialog: doOK";
    accept();
}
void options_dialog::doCancel()
{
    qDebug() << "options_dialog: doCancel";
    reject();
}

void options_dialog::doAccept()
{
    qDebug() << "options_dialog: doAccept";
    save();
}

void options_dialog::doReject()
{
    qDebug() << "options_dialog: doReject";
    reload();
}

options_dialog::~options_dialog()
{
    if (tracker_dialog)
        tracker_dialog->unregister_tracker();
    if (proto_dialog)
        proto_dialog->unregister_protocol();
    if (filter_dialog)
        filter_dialog->unregister_filter();
}
