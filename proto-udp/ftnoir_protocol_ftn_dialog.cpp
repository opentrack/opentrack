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
#include "ftnoir_protocol_ftn.h"
#include "api/plugin-api.hpp"

FTNControls::FTNControls()
{
    ui.setupUi( this );

    tie_setting(s.ip1, ui.spinIPFirstNibble);
    tie_setting(s.ip2, ui.spinIPSecondNibble);
    tie_setting(s.ip3, ui.spinIPThirdNibble);
    tie_setting(s.ip4, ui.spinIPFourthNibble);
    tie_setting(s.port, ui.spinPortNumber);

    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &FTNControls::doOK);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &FTNControls::doCancel);
}

//
// OK clicked on server-dialog
//
void FTNControls::doOK() {
    s.b->save();
    close();
}

//
// Cancel clicked on server-dialog
//
void FTNControls::doCancel()
{
    close();
}
