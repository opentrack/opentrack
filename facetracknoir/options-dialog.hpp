#pragma once

#include <QObject>
#include <QWidget>
#include "ui_settings.h"
#include "opentrack/shortcuts.h"
#include "ftnoir_tracker_pt/ftnoir_tracker_pt_settings.h"

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
    settings_pt pt;
private slots:
    void doOK();
    void doCancel();
};
