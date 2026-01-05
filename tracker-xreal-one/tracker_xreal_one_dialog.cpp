/**
 * Copyright (C) 2026 DimKa <xstuff88@gmail.com>
 *
 * The MIT License (MIT)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”),
 * to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "tracker_xreal_one.h"
#include "api/plugin-api.hpp"

XRealOneSettingsDialog::XRealOneSettingsDialog()
{
    ui.setupUi( this );

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(doOK()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(doCancel()));

    tie_setting(s.port, ui.spinPortNumber);
    tie_setting(s.ip_address, ui.lineEditIPAddress);
    tie_setting(s.g_constant, ui.spinGConstant);
    tie_setting(s.header_value, ui.lineEditHeaderValue);
    tie_setting(s.ga_marker_value, ui.lineEditDataTypeValue);
    tie_setting(s.use_fusion_offset, ui.checkBoxUseFusionOffset);
}

void XRealOneSettingsDialog::doOK()
{
    s.changed = true;
    s.b->save();
    close();
}

void XRealOneSettingsDialog::doCancel()
{
    close();
}

