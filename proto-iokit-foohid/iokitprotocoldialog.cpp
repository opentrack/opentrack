/* Copyright (c) 2017, Eike Ziller                                               *
 *                                                                               *
 * Permission to use, copy, modify, and/or distribute this software for any      *
 * purpose with or without fee is hereby granted, provided that the above        *
 * copyright notice and this permission notice appear in all copies.             *
 */

#include "iokitprotocoldialog.h"

#include <QDialogButtonBox>
#include <QLabel>
#include <QVBoxLayout>

IOKitProtocolDialog::IOKitProtocolDialog()
{
    setLayout(new QVBoxLayout);
    layout()->addWidget(new QLabel(tr("No settings available.")));
    layout()->addItem(new QSpacerItem(0, 0, QSizePolicy::Ignored, QSizePolicy::Expanding));
    auto buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok);
    layout()->addWidget(buttonBox);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
}

void IOKitProtocolDialog::register_protocol(IProtocol *)
{
}

void IOKitProtocolDialog::unregister_protocol()
{
}
