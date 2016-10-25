/* Copyright (c) 2014-2015, Stanislaw Halik <sthalik@misaki.pl>

 * Permission to use, copy, modify, and/or distribute this
 * software for any purpose with or without fee is hereby granted,
 * provided that the above copyright notice and this permission
 * notice appear in all copies.
 */

#include "mapping-window.hpp"
#include "logic/main-settings.hpp"
#include "spline-widget/spline-widget.hpp"
MapWidget::MapWidget(Mappings& m) :
    m(m)
{
    ui.setupUi(this);

    reload();

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.a_x.altp, ui.tx_altp);
    tie_setting(s.a_y.altp, ui.ty_altp);
    tie_setting(s.a_z.altp, ui.tz_altp);
    tie_setting(s.a_yaw.altp, ui.rx_altp);
    tie_setting(s.a_pitch.altp, ui.ry_altp);
    tie_setting(s.a_roll.altp, ui.rz_altp);
}

void MapWidget::reload()
{

    struct {
        spline_widget* qfc;
        Axis axis;
        QCheckBox* checkbox;
        bool altp;
    } qfcs[] =
    {
    { ui.rxconfig, Yaw,   nullptr, false },
    { ui.ryconfig, Pitch, nullptr, false },
    { ui.rzconfig, Roll,  nullptr, false },
    { ui.txconfig, TX,    nullptr, false },
    { ui.tyconfig, TY,    nullptr, false },
    { ui.tzconfig, TZ,    nullptr, false },

    { ui.rxconfig_alt, Yaw,   ui.rx_altp, true },
    { ui.ryconfig_alt, Pitch, ui.ry_altp, true },
    { ui.rzconfig_alt, Roll,  ui.rz_altp, true },
    { ui.txconfig_alt, TX,    ui.tx_altp, true },
    { ui.tyconfig_alt, TY,    ui.ty_altp, true },
    { ui.tzconfig_alt, TZ,    ui.tz_altp, true },
    { nullptr, Yaw, nullptr, false }
};

    for (int i = 0; qfcs[i].qfc; i++)
    {
        const bool altp = qfcs[i].altp;

        Map& axis = m(qfcs[i].axis);
        spline& conf = altp ? axis.spline_alt : axis.spline_main;
        //const QString& name = altp ? axis.name2 : axis.name1;
        //conf.set_bundle(make_bundle(name));
        qfcs[i].qfc->setConfig(&conf);

        if (altp)
        {
            spline_widget& qfc = *qfcs[i].qfc;
            connect(qfcs[i].checkbox, &QCheckBox::toggled,
                    this,
                    [&](bool f) -> void {qfc.setEnabled(f); qfc.force_redraw();});
            qfc.setEnabled(qfcs[i].checkbox->isChecked());
            qfc.force_redraw();
        }

        if (qfcs[i].axis >= 3)
            qfcs[i].qfc->set_snap(1, 2.5);
        else
            qfcs[i].qfc->set_snap(.5, 1);
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
