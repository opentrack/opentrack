#pragma once

/* Copyright (c) 2015, 2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "export.hpp"

#ifdef _WIN32
#   include "logic/win32-shortcuts.h"
#   include "dinput/keybinding-worker.hpp"
#endif

#include "ui_keyboard_listener.h"

#include <QDialog>
#include <QtEvents>

class OTR_GUI_EXPORT keyboard_listener : public QDialog
{
    Q_OBJECT
    Ui_keyboard_listener ui;
#ifdef _WIN32
    KeybindingWorker::Token token;
#endif
public:
    keyboard_listener(QWidget* parent = nullptr);
#ifndef _WIN32
    void keyPressEvent(QKeyEvent* event) override;
#endif
signals:
    void key_pressed(QKeySequence k);
    void joystick_button_pressed(QString guid, int idx, bool held);
};
