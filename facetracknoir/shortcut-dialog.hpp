#pragma once

#include <QObject>
#include <QWidget>
#include "ui_keyboard.h"
#include "opentrack/shortcuts.h"

class KeyboardShortcutDialog: public QWidget
{
    Q_OBJECT
signals:
    void reload();
public:
    KeyboardShortcutDialog();
private:
    Ui::UICKeyboardShortcutDialog ui;
    Shortcuts::settings s;
private slots:
    void doOK();
    void doCancel();
};
