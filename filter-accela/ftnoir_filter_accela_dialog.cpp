/* Copyright (c) 2012-2015 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "ftnoir_filter_accela.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "api/plugin-api.hpp"
#include "spline/spline-widget.hpp"
#include <QDialog>

dialog_accela::dialog_accela()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    connect(&s.rot_sensitivity, SIGNAL(valueChanged(const slider_value&)), this, SLOT(update_rot_display(const slider_value&)));
    connect(&s.pos_sensitivity, SIGNAL(valueChanged(const slider_value&)), this, SLOT(update_pos_display(const slider_value&)));
    connect(&s.ewma, SIGNAL(valueChanged(const slider_value&)), this, SLOT(update_ewma_display(const slider_value&)));
    connect(&s.rot_deadzone, SIGNAL(valueChanged(const slider_value&)), this, SLOT(update_rot_dz_display(const slider_value&)));
    connect(&s.pos_deadzone, SIGNAL(valueChanged(const slider_value&)), this, SLOT(update_pos_dz_display(const slider_value&)));
    connect(&s.rot_nonlinearity, SIGNAL(valueChanged(const slider_value&)), this, SLOT(update_rot_nl_slider(const slider_value&)));

    tie_setting(s.rot_sensitivity, ui.rotation_slider);
    tie_setting(s.pos_sensitivity, ui.translation_slider);
    tie_setting(s.ewma, ui.ewma_slider);
    tie_setting(s.rot_deadzone, ui.rot_dz_slider);
    tie_setting(s.pos_deadzone, ui.trans_dz_slider);
    tie_setting(s.rot_nonlinearity, ui.rot_nl_slider);

    update_rot_display(s.rot_sensitivity);
    update_pos_display(s.pos_sensitivity);
    update_ewma_display(s.ewma);
    update_rot_dz_display(s.rot_deadzone);
    update_pos_dz_display(s.pos_deadzone);
    update_rot_nl_slider(s.rot_nonlinearity);

//#define SPLINE_ROT_DEBUG
//#define SPLINE_TRANS_DEBUG

#if defined SPLINE_ROT_DEBUG || defined SPLINE_TRANS_DEBUG
    {
        spline rot, trans;
        s.make_splines(rot, trans);
        QDialog dr, dt;
        spline_widget r(&dr);
        spline_widget t(&dt);
        dr.setWindowTitle("Accela rotation gain"); r.set_preview_only(true); r.setEnabled(false);
        r.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); r.setConfig(&rot);
        dt.setWindowTitle("Accela translation gain"); t.set_preview_only(true); t.setEnabled(false);
        r.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); t.setConfig(&trans);
        r.setFixedSize(1024, 600); t.setFixedSize(1024, 600);

#ifdef SPLINE_ROT_DEBUG
        dr.show();
#endif

#ifdef SPLINE_TRANS_DEBUG
        dt.show();
#endif

        if (dr.isVisible())
            dr.exec();
        if (dt.isVisible())
            dt.exec();
    }
#endif
}

void dialog_accela::doOK()
{
    save();
    close();
}

void dialog_accela::doCancel()
{
    close();
}

void dialog_accela::save()
{
    s.b->save();
}

#define FIELD(x, a) ((a).arg(double((x)), 0, 'g', 4))
#define LIT(x) QStringLiteral(x)

void dialog_accela::update_rot_display(const slider_value& val)
{
    static const QString str(QString::fromUtf8("%1°"));
    ui.rot_gain->setText(FIELD(val, str));
}

void dialog_accela::update_pos_display(const slider_value& val)
{
    ui.trans_gain->setText(FIELD(val, LIT("%1mm")));
}

void dialog_accela::update_ewma_display(const slider_value& val)
{
    ui.ewma_label->setText(FIELD(val, LIT("%1ms")));
}

void dialog_accela::update_rot_dz_display(const slider_value& val)
{
    static const QString str(QString::fromUtf8("%1°"));
    ui.rot_dz->setText(FIELD(val, str));
}

void dialog_accela::update_pos_dz_display(const slider_value& val)
{
    ui.trans_dz->setText(FIELD(val, LIT("%1mm")));
}

void dialog_accela::update_rot_nl_slider(const slider_value& val)
{
    ui.rot_nl->setText(FIELD(val, LIT(
                        "<html><head/><body>"
                        "<p>x<span style='vertical-align:super;'>"
                        "%1"
                        "</span></p>"
                        "</body></html>")));
}

