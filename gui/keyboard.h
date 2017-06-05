#pragma once

#ifdef _WIN32
#   include "logic/win32-shortcuts.h"
#   include "dinput/keybinding-worker.hpp"
#endif

#include "ui_keyboard_listener.h"

#include <QDialog>
#include <QKeyEvent>

class KeyboardListener : public QDialog
{
    Q_OBJECT
    Ui_keyboard_listener ui;
#ifdef _WIN32
    KeybindingWorker::Token token;
#endif
public:
    KeyboardListener(QWidget* parent = nullptr);
#ifndef _WIN32
    void keyPressEvent(QKeyEvent* event) override;
#endif
signals:
    void key_pressed(QKeySequence k);
    void joystick_button_pressed(QString guid, int idx, bool held);
};
