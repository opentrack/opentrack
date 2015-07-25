#pragma once

#include <QObject>
#include <QWidget>
#include "ui_settings.h"
#include "opentrack/shortcuts.h"

class OptionsDialog: public QWidget
{
    Q_OBJECT
signals:
    void reload();
public:
    OptionsDialog();
private:
    Ui::UI_Settings ui;
    Shortcuts::settings s;
private slots:
    void doOK();
    void doCancel();
    void bind_key(value<int>& ret, QLabel* label);
};
