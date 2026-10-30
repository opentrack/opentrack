/* Copyright (c) 2026, Adrian Lopez <adrianlopezroche@gmail.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */
#include "papertracker-help-dialog.h"
#include "ui_papertracker-help.h"
#include "localizedfilename.h"
#include <QIODevice>
#include <QFile>
#include <QRegularExpression>

PaperTrackerHelpDialog::PaperTrackerHelpDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::PaperTrackerHelpDialog)
{
    ui->setupUi(this);

    const auto filename = papertracker::localized_filename(":html", "papertracker-help.html");
    if (!filename.isEmpty())
        loadHelpText(filename);
}

PaperTrackerHelpDialog::~PaperTrackerHelpDialog()
{
    delete ui;
}

/* Load help text from filePath into QTextBrowser control.
*/
void PaperTrackerHelpDialog::loadHelpText(const QString &filePath)
{
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        ui->tbHelpText->setPlainText(
            QString("Error: Could not open help file:\n%1").arg(filePath)
        );
        return;
    }

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);
    QString html = in.readAll();
    file.close();

    ui->tbHelpText->setHtml(removeSpaces(html));
}

/* Extra whitespace casuses formatting issues with QTextBrowser, so remove it.
*/
QString PaperTrackerHelpDialog::removeSpaces(QString &input)
{
    // Collapse all whitespace sequences to a single space.
    input.replace(QRegularExpression("\\s+"), " ");

    // Remove any remaining spaces between tags.
    input.replace(QRegularExpression(">\\s+<"), "><");

    // Trim leading/trailing whitespace.
    return input.trimmed();
}