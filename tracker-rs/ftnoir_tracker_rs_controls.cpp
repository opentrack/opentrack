/*
 * Copyright (c) 2015, Intel Corporation
 *   Author: Xavier Hallade <xavier.hallade@intel.com>
 * Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "ftnoir_tracker_rs_controls.h"

RSdialog_realsense::RSdialog_realsense()
{
    ui.setupUi(this);
    connect(ui.buttonBox, &QDialogButtonBox::accepted, this, &QDialog::close);
    connect(ui.buttonBox, &QDialogButtonBox::rejected, this, &QDialog::close);
}

void RSdialog_realsense::set_buttons_visible(bool x)
{
    ui.buttonBox->setVisible(x);
}

void RSdialog_realsense::doOK()
{
    close();
}

void RSdialog_realsense::doCancel()
{
    close();
}
