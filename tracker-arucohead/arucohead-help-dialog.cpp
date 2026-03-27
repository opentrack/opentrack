/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "arucohead-help-dialog.h"
#include "ui_arucohead-help.h"

ArucoheadHelpDialog::ArucoheadHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ArUcoheadHelpDialog)
{
    ui->setupUi(this);
}

ArucoheadHelpDialog::~ArucoheadHelpDialog()
{
    delete ui;
}