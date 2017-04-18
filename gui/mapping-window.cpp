/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "mapping-window.hpp"
#include "logic/main-settings.hpp"
#include "spline/spline-widget.hpp"

MapWidget::MapWidget(Mappings& m) : m(m)
{
    ui.setupUi(this);

    load();

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.a_yaw.altp, ui.rx_altp);
    tie_setting(s.a_pitch.altp, ui.ry_altp);
    tie_setting(s.a_roll.altp, ui.rz_altp);
    tie_setting(s.a_x.altp, ui.tx_altp);
    tie_setting(s.a_y.altp, ui.ty_altp);
    tie_setting(s.a_z.altp, ui.tz_altp);

    tie_setting(s.a_yaw.clamp, ui.max_yaw_rotation);
    tie_setting(s.a_pitch.clamp, ui.max_pitch_rotation);
    tie_setting(s.a_roll.clamp, ui.max_roll_rotation);
    tie_setting(s.a_x.clamp, ui.max_x_translation);
    tie_setting(s.a_y.clamp, ui.max_y_translation);
    tie_setting(s.a_z.clamp, ui.max_z_translation);
}

void MapWidget::load()
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

    for (QComboBox* x : { ui.max_yaw_rotation, ui.max_pitch_rotation, ui.max_roll_rotation })
        for (a y : { a::r180, a::r90, a::r60, a::r45, a::r30, a::r25, a::r20, a::r15, a::r10 })
            x->addItem(QString::number(y) + "°", y);

    for (QComboBox* x : { ui.max_x_translation, ui.max_y_translation, ui.max_z_translation })
        for (a y : { a::t30, a::t20, a::t15, a::t10, a::t100 })
            x->addItem(QStringLiteral("%1 cm").arg(int(y)), y);

    for (int i = 0; qfcs[i].qfc; i++)
    {
        const bool altp = qfcs[i].altp;

        Map& axis = m(qfcs[i].axis);
        spline& conf = altp ? axis.spline_alt : axis.spline_main;
        spline_widget& qfc = *qfcs[i].qfc;

        if (altp)
        {
            connect(&axis.opts.altp,
                    static_cast<void(base_value::*)(bool) const>(&base_value::valueChanged),
                    this,
                    [&](bool f) -> void {qfc.setEnabled(f); qfc.force_redraw();});
            qfc.setEnabled(axis.opts.altp);
            qfc.force_redraw();
        }

        connect(&axis.opts.clamp, static_cast<void(base_value::*)(int) const>(&base_value::valueChanged),
                &qfc, [i, &conf, &qfc](int value) {
            conf.set_max_input(value);
            qfc.reload_spline();
            qfc.set_x_step(value + 1e-2 >= 90 ? 10 : 5);

            if (i >= 3)
                qfc.set_snap(1, 2.5);
            else
            {
                const double x_snap = std::fmax(.5, conf.max_input() / 100.);
                qfc.set_snap(x_snap, 1);
            }
        });
        // force signal to avoid duplicating the slot's logic
        qfcs[i].qfc->setConfig(&conf);
        axis.opts.clamp.valueChanged(axis.opts.clamp);
    }

}

void MapWidget::closeEvent(QCloseEvent*)
{
    invalidate_dialog();
}

void MapWidget::save_dialog()
{
    mem<QSettings> settings_ = group::ini_file();
    QSettings& settings = *settings_;

    s.b_map->save_deferred(settings);

    for (int i = 0; i < 6; i++)
    {
        m.forall([&](Map& s)
        {
            s.spline_main.save(settings);
            s.spline_alt.save(settings);
            s.opts.b_mapping_window->save_deferred(settings);
        });
    }
}

void MapWidget::invalidate_dialog()
{
    s.b_map->reload();

    m.forall([](Map& s)
    {
        s.spline_main.reload();
        s.spline_alt.reload();
        s.opts.b_mapping_window->reload();
    });
}

void MapWidget::doOK()
{
    save_dialog();
    close();
}

void MapWidget::doCancel()
{
    invalidate_dialog();
    close();
}
