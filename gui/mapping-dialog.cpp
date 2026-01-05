/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "mapping-dialog.hpp"
#include "logic/main-settings.hpp"
#include "spline/spline-widget.hpp"

#include <QtEvents>

using namespace options;

mapping_dialog::mapping_dialog(Mappings& m) : m(m), widgets{}
{
    ui.setupUi(this);

    QWidget* pages[] = {
        ui.tabWidgetPage1,
        ui.tabWidgetPage2,
        ui.tabWidgetPage3,
        ui.tabWidgetPage4,
        ui.tabWidgetPage5,
        ui.tabWidgetPage6,
    };

    {
        QColor bg = palette().window().color();

        QString tmp;
        tmp.reserve(32);

        tmp += QStringLiteral(".QWidget { background-color: rgb(%1, %2, %3); } ")
            .arg(bg.red()).arg(bg.green()).arg(bg.blue());

        for (QWidget* w : pages)
            w->setStyleSheet(tmp);
    }

    load();

    tie_setting(s.a_yaw.altp, ui.rx_altp);
    tie_setting(s.a_pitch.altp, ui.ry_altp);
    tie_setting(s.a_roll.altp, ui.rz_altp);
    tie_setting(s.a_x.altp, ui.tx_altp);
    tie_setting(s.a_y.altp, ui.ty_altp);
    tie_setting(s.a_z.altp, ui.tz_altp);

    tie_setting(s.a_yaw.clamp_x_, ui.max_yaw_rotation);
    tie_setting(s.a_pitch.clamp_x_, ui.max_pitch_rotation);
    tie_setting(s.a_roll.clamp_x_, ui.max_roll_rotation);
    tie_setting(s.a_x.clamp_x_, ui.max_x_translation);
    tie_setting(s.a_y.clamp_x_, ui.max_y_translation);
    tie_setting(s.a_z.clamp_x_, ui.max_z_translation);

    tie_setting(s.a_x.clamp_y_, ui.max_x_out);
    tie_setting(s.a_y.clamp_y_, ui.max_y_out);
    tie_setting(s.a_z.clamp_y_, ui.max_z_out);

    tie_setting(s.a_pitch.clamp_y_, ui.max_pitch_output);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &mapping_dialog::accept);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &mapping_dialog::reject);
    connect(this, &mapping_dialog::accepted, this, &mapping_dialog::doAccept);
    connect(this, &mapping_dialog::rejected, this, &mapping_dialog::doReject);
}

void mapping_dialog::load()
{
    struct {
        spline_widget* qfc;
        Axis axis;
        QCheckBox* checkbox;
        bool altp;
    } qfcs[] =
    {
        { ui.rxconfig, Yaw,   nullptr, false, },
        { ui.ryconfig, Pitch, nullptr, false, },
        { ui.rzconfig, Roll,  nullptr, false, },
        { ui.txconfig, TX,    nullptr, false, },
        { ui.tyconfig, TY,    nullptr, false, },
        { ui.tzconfig, TZ,    nullptr, false, },
        { ui.rxconfig_alt, Yaw,   ui.rx_altp, true, },
        { ui.ryconfig_alt, Pitch, ui.ry_altp, true, },
        { ui.rzconfig_alt, Roll,  ui.rz_altp, true, },
        { ui.txconfig_alt, TX,    ui.tx_altp, true, },
        { ui.tyconfig_alt, TY,    ui.ty_altp, true, },
        { ui.tzconfig_alt, TZ,    ui.tz_altp, true, },
        { nullptr, Yaw, nullptr, false }
    };

    using a = axis_opts::max_clamp;

    ui.max_pitch_output->setItemData(0, int(a::o_r180));
    ui.max_pitch_output->setItemData(1, int(a::o_r90));

    for (QComboBox* x : { ui.max_yaw_rotation, ui.max_pitch_rotation, ui.max_roll_rotation })
        for (a y : { a::r180, a::r90, a::r60, a::r45, a::r30, a::r25, a::r20, a::r15, a::r10 })
            x->addItem(tr("%1°").arg(y), y);

    for (QComboBox* x : { ui.max_x_translation, ui.max_y_translation, ui.max_z_translation })
        for (a y : { a::t30, a::t20, a::t15, a::t10, a::t75, a::t100, a::t150, a::t300, a::t600 })
            x->addItem(tr("%1 cm").arg(int(y)), y);

    for (QComboBox* x : { ui.max_x_out, ui.max_y_out, ui.max_z_out })
        for (a y : { a::o_t75, a::o_t100, a::o_t150, a::o_t300, a::o_t600 })
            x->addItem(tr("%1 cm").arg(abs(int(y))), y);

    for (int i = 0; qfcs[i].qfc; i++)
    {
        const bool altp = qfcs[i].altp;

        Map& axis = m(qfcs[i].axis);
        spline& conf = altp ? axis.spline_alt : axis.spline_main;
        spline_widget& qfc = *qfcs[i].qfc;

        if (altp)
        {
            connect(&axis.opts.altp,
                    value_::value_changed<bool>(),
                    this, [&](bool f) { qfc.setEnabled(f); });
            qfc.setEnabled(axis.opts.altp);
        }

        auto update_xstep = [&qfc](int clamp_x) {
            int value;

            if (clamp_x <= a::r30)
                value = 1;
            else if (clamp_x <= a::r45)
                value = 5;
            else if (clamp_x <= a::t150)
                value = 10;
            else if (clamp_x <= a::t300)
                value = 25;
            else
                value = 50;

            qfc.set_x_step(value);
        };

        auto update_ystep = [&qfc](int clamp_y) {
            int value;
            switch (clamp_y)
            {
            default:
            case a::o_r180:
                value = 20; break;
            case a::o_r90:
                value = 10; break;
            case a::o_t75:
                value = 5; break;
            case a::o_t100:
            case a::o_t150:
                value = 10; break;
            case a::o_t300:
                value = 50; break;
            case a::o_t600:
                value = 100; break;
            }
            qfc.set_y_step(value);
        };

        qfc.set_snap(.5, 1);

        connect(&axis.opts.clamp_x_, value_::value_changed<int>(), &qfc, update_xstep);
        connect(&axis.opts.clamp_y_, value_::value_changed<int>(), &qfc, update_ystep);

        // force signal to avoid duplicating the slot's logic
        qfc.set_config(&conf);

        update_xstep(axis.opts.clamp_x_);
        update_ystep(axis.opts.clamp_y_);

        widgets[i % 6][altp ? 1 : 0] = &qfc;
    }
}

void mapping_dialog::refresh_tab()
{
    if (!isVisible())
        return;

    const int idx = ui.tabWidget->currentIndex();

    if (idx >= 0 && idx < 6) [[likely]]
    {
        widgets[idx][0]->repaint();
        widgets[idx][1]->repaint();
    }
    else
        qDebug() << "mapping_dialog: bad index" << idx;
}

void mapping_dialog::save_dialog()
{
    s.b_map->save();

    for (int i = 0; i < 6; i++)
    {
        m.forall([&](Map& s)
        {
            s.spline_main.save();
            s.spline_alt.save();
            s.opts.b_mapping_window->save();
        });
    }
}

void mapping_dialog::invalidate_dialog()
{
    s.b_map->reload();

    m.forall([](Map& s)
    {
        s.spline_main.reload();
        s.spline_alt.reload();
        s.opts.b_mapping_window->reload();
    });
}

void mapping_dialog::closeEvent(QCloseEvent *)
{
    qDebug() << "mapping_dialog: closeEvent";
    reject();
}

void mapping_dialog::doOK()
{
    qDebug() << "mapping_dialog: doOK";
    accept();
}

void mapping_dialog::doCancel()
{
    qDebug() << "mapping_dialog: doCancel";
    reject();
}

void mapping_dialog::doAccept()
{
    qDebug() << "mapping_dialog: doAccept";
    save_dialog();
}

void mapping_dialog::doReject()
{
    qDebug() << "mapping_dialog: doReject";
    invalidate_dialog();
}
