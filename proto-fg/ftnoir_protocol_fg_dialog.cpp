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
#include "ftnoir_protocol_fg.h"
#include <QObject>
#include <QFile>
#include "api/plugin-api.hpp"

//*******************************************************************************************************
// FaceTrackNoIR Client Settings-dialog.
//*******************************************************************************************************

//
// Constructor for server-settings-dialog
//
FGControls::FGControls()
{
    ui.setupUi( this );

    tie_setting(s.ip1, ui.spinIPFirstNibble);
    tie_setting(s.ip2, ui.spinIPSecondNibble);
    tie_setting(s.ip3, ui.spinIPThirdNibble);
    tie_setting(s.ip4, ui.spinIPFourthNibble);
    tie_setting(s.port, ui.spinPortNumber);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
}

void FGControls::doOK() {
    s.b->save();
    close();
}

void FGControls::doCancel() {
    close();
}

