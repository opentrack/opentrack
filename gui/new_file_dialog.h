#pragma once

#include "ui_new_config.h"
#include "options/options.hpp"
#include <QFile>
#include <QRegExp>
#include <QString>
#include <QMessageBox>

class new_file_dialog : public QDialog
{
    Q_OBJECT
public:
    new_file_dialog(QWidget* parent = 0) : QDialog(parent), ok(false)
    {
        ui.setupUi(this);
        connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(ok_clicked()));
        connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(cancel_clicked()));
        setFixedSize(size());
    }
    bool is_ok(QString& name_)
    {
        name_ = name;
        return ok;
    }
private:
    Ui::UI_new_config ui;
    bool ok;
    QString name;
private slots:
    void cancel_clicked() { close(); }
    void ok_clicked()
    {
        QString text = ui.lineEdit->text();
        text = text.replace('/', "");
        text = text.replace('\\', "");
        if (text != "" && !text.endsWith(".ini"))
            text += ".ini";
        if (text == "" || text == ".ini" || QFile(options::group::ini_directory() + "/" + text).exists())
        {
            QMessageBox::warning(this,
                                 "File exists", "This file already exists. Pick another name.",
                                 QMessageBox::Ok, QMessageBox::NoButton);
            return;
        }
        ok = true;
        close();
        name = text;
    }
};
