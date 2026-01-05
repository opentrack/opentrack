#pragma once

#include "ui_new_config.h"
#include "options/options.hpp"
#include <QFile>
#include <QString>
#include <QMessageBox>

class new_file_dialog : public QDialog
{
    Q_OBJECT
public:
    new_file_dialog(QWidget* parent = nullptr);
    bool is_ok(QString& name_);

private:
    Ui::UI_new_config ui;
    bool ok = false;
    QString name;

private slots:
    void cancel_clicked();
    void ok_clicked();
};
