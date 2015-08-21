/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs_controls.h"

#include <QProcess>
#include <QMessageBox>

RSTrackerControls::RSTrackerControls()
{
    ui.setupUi(this);
    connect(ui.triggerSDKInstallButton, SIGNAL(clicked(bool)), this, SLOT(doInstallRSRuntime()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));
    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
}

void RSTrackerControls::doInstallRSRuntime()
{
    bool processStarted = QProcess::startDetached("clientfiles\\intel_rs_sdk_runtime_websetup_6.0.21.6598.exe --finstall=core,face3d --fnone=all");
    if(processStarted){
        this->close();
    }
    else{
        QMessageBox::warning(0, "Intel® RealSenseTM Runtime Installation", "Installation process failed to start.", QMessageBox::Ok);
    }
}

void RSTrackerControls::doOK()
{
    this->close();
}

void RSTrackerControls::doCancel()
{
    this->close();
}
