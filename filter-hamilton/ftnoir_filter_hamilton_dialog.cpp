/* Copyright (c) 2020, GO63-samara <go1@list.ru> 
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "ftnoir_filter_hamilton.h"
#include <cmath>
#include <QDebug>
#include <QString>
#include "api/plugin-api.hpp"
#include "ui_ftnoir_hamilton_filtercontrols.h"

dialog_hamilton::dialog_hamilton()
{
    ui.setupUi(this);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.kMaxRot, ui.maxRot);
    tie_setting(s.kMaxRot, ui.lbMaxRot, [](double x)
      { return QStringLiteral("%1\xB0").arg(x, 0, 'f', 2);});

    tie_setting(s.kPowRot, ui.powRot);
    tie_setting(s.kPowRot, ui.lbPowRot, [](double x)
      { return QStringLiteral("%1").arg(x, 0, 'f', 2);});

    tie_setting(s.kDeadZoneRot, ui.dzRot);
    tie_setting(s.kDeadZoneRot, ui.lbDZRot, [](double x)
      { return QStringLiteral("%1\xB0").arg(x, 0, 'f', 2);});

    tie_setting(s.kPowZoom, ui.powZoom);
    tie_setting(s.kPowZoom, ui.lbPowZoom, [](double x)
      { return QStringLiteral("%1").arg(x, 0, 'f', 2);});

    tie_setting(s.kMaxZ, ui.maxZ);
    tie_setting(s.kMaxZ, ui.lbMaxZ, [](double x)
      { return QStringLiteral("%1").arg(x, 0, 'f', 2);});
			
    tie_setting(s.kMaxDist, ui.maxDist);
    tie_setting(s.kMaxDist, ui.lbMaxDist, [](double x)
      { return QStringLiteral("%1cm").arg(x, 0, 'f', 2);});

    tie_setting(s.kPowDist, ui.powDist);
    tie_setting(s.kPowDist, ui.lbPowDist, [](double x)
      { return QStringLiteral("%1").arg(x, 0, 'f', 2);});

    tie_setting(s.kDeadZoneDist, ui.dzDist);
    tie_setting(s.kDeadZoneDist, ui.lbDZDist, [](double x)
      { return QStringLiteral("%1cm").arg(x, 0, 'f', 2);});
}

void dialog_hamilton::doOK()
{
    s.b->save();
    close();
}

void dialog_hamilton::doCancel()
{
    close();
}
