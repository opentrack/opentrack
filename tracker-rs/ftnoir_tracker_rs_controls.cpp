/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs_controls.h"

RSTrackerControls::RSTrackerControls()
{
    ui.setupUi(this);
    connect(ui.triggerSDKInstallButton, SIGNAL(clicked(bool)), this, SLOT(doInstallRSRuntime()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
}

void RSTrackerControls::doInstallRSRuntime()
{
    bool pStarted = RSTracker::startSdkInstallationProcess();
    if(pStarted == true)
        close();
}

void RSTrackerControls::doOK()
{
    close();
}

void RSTrackerControls::doCancel()
{
    close();
}
