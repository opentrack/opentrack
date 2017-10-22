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

    tie_setting(s.rot_sensitivity, ui.rotation_slider);
    tie_setting(s.pos_sensitivity, ui.translation_slider);
    tie_setting(s.ewma, ui.ewma_slider);
    tie_setting(s.rot_deadzone, ui.rot_dz_slider);
    tie_setting(s.pos_deadzone, ui.trans_dz_slider);

    tie_setting(s.rot_sensitivity, ui.rot_gain, [](const slider_value& s) { return tr("%1°").arg(s, 0, 'g', 4); });
    tie_setting(s.pos_sensitivity, ui.trans_gain, [](const slider_value& s) { return tr("%1mm").arg(s, 0, 'g', 4); });
    tie_setting(s.ewma, ui.ewma_label, [](const slider_value& s) { return tr("%1ms").arg(s); });
    tie_setting(s.rot_deadzone, ui.rot_dz, [](const slider_value& s) { return tr("%1°").arg(s, 0, 'g', 4); });
    tie_setting(s.pos_deadzone, ui.trans_dz, [](const slider_value& s) { return tr("%1mm").arg(s); });

//#define SPLINE_ROT_DEBUG
//#define SPLINE_TRANS_DEBUG

#if defined SPLINE_ROT_DEBUG || defined SPLINE_TRANS_DEBUG
    {
        spline rot, pos;
        s.make_splines(rot, pos);

#ifdef SPLINE_ROT_DEBUG
        QDialog dr;
        spline_widget r(&dr);
        dr.setWindowTitle("Accela rotation gain"); r.set_preview_only(true); r.setEnabled(true);
        r.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); r.setConfig(&rot);
        r.setFixedSize(1024, 600);
        dr.show();
        dr.exec();
#endif

#ifdef SPLINE_TRANS_DEBUG
        QDialog dt;
        spline_widget t(&dt);
        dt.setWindowTitle("Accela translation gain"); t.set_preview_only(true); t.setEnabled(true);
        dt.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); t.setConfig(&pos);
        t.setFixedSize(1024, 600);
        dt.show();
        dt.exec();
#endif
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



