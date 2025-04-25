/* Copyright (c) 2012-2015 Stanislaw Halik
 * Copyright (c) 2023-2024 Michael Welter
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "accela_hamilton.h"
#include <cmath>
#include <QDebug>
#include <algorithm>
#include <QDoubleSpinBox>
#include "api/plugin-api.hpp"
#include "spline/spline-widget.hpp"
#include <QDialog>

using namespace options;

dialog_accela_hamilton::dialog_accela_hamilton()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.rot_smoothing, ui.rotation_slider);
    tie_setting(s.pos_smoothing, ui.translation_slider);
    tie_setting(s.rot_deadzone, ui.rot_dz_slider);
    tie_setting(s.pos_deadzone, ui.trans_dz_slider);

    tie_setting(s.rot_smoothing, ui.rot_gain, [](const slider_value& s) { return tr("%1°").arg(s.cur(), 0, 'g', 4); });
    tie_setting(s.pos_smoothing, ui.trans_gain, [](const slider_value& s) { return tr("%1mm").arg(s.cur(), 0, 'g', 4); });
    tie_setting(s.rot_deadzone, ui.rot_dz, [](const slider_value& s) { return tr("%1°").arg(s.cur(), 0, 'g', 4); });
    tie_setting(s.pos_deadzone, ui.trans_dz, [](const slider_value& s) { return tr("%1mm").arg(s.cur()); });

    tie_setting(s.max_zoomed_smoothing, ui.max_zoomed_smoothing);
    tie_setting(s.max_zoomed_smoothing, ui.lb_max_zoomed_smoothing, [](double x)
      { return tr("%1°").arg(x, 0, 'g', 3);});

    tie_setting(s.max_z, ui.max_z);
    tie_setting(s.max_z, ui.lb_max_z, [](double x)
      { return tr("%1mm").arg(x, 0, 'g', 3);});

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
        r.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); r.set_config(&rot);
        r.setFixedSize(1024, 600);
        dr.show();
        dr.exec();
#endif

#ifdef SPLINE_TRANS_DEBUG
        QDialog dt;
        spline_widget t(&dt);
        dt.setWindowTitle("Accela translation gain"); t.set_preview_only(true); t.setEnabled(true);
        dt.setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed); t.set_config(&pos);
        t.setFixedSize(1024, 600);
        dt.show();
        dt.exec();
#endif
    }
#endif
}

void dialog_accela_hamilton::doOK()
{
    save();
    close();
}

void dialog_accela_hamilton::doCancel()
{
    close();
}

void dialog_accela_hamilton::save()
{
    s.b->save();
}

void dialog_accela_hamilton::reload()
{
    s.b->reload();
}

void dialog_accela_hamilton::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
}
