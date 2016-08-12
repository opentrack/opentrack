/* Homepage         http://facetracknoir.sourceforge.net/home/default.htm        *
 *                                                                               *
 * ISC License (ISC)                                                             *
 *                                                                               *
 * Copyright (c) 2015, Wim Vriend                                                *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */
#include "ftnoir_protocol_sc.h"
#include <QDebug>
#include "api/plugin-api.hpp"

SCControls::SCControls()
{
    ui.setupUi( this );

    // Connect Qt signals to member-functions
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.sxs_manifest, ui.comboBox);
}

void SCControls::doOK() {
    s.b->save();
    close();
}

void SCControls::doCancel()
{
    close();
}

