#pragma once

/* Copyright (c) 2015, 2017 Stanislaw Halik
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 */

#include "export.hpp"

#ifdef _WIN32
#   include "input/keybinding-worker.hpp"
#endif

#include "ui_listener.h"

#include <QDialog>
#include <QtEvents>

class OTR_GUI_EXPORT keyboard_listener : public QDialog
{
    Q_OBJECT

#ifdef _WIN32
    void receive_key(const Key& k);

    KeybindingWorker::Token token{[this](const Key& k) {receive_key(k);}};
#else
    void keyPressEvent(QKeyEvent* event) override;
#endif

    Ui_keyboard_listener ui;

public:
    keyboard_listener(QWidget* parent = nullptr);
signals:
    void key_pressed(QKeySequence k);
    void joystick_button_pressed(QString guid, int idx, bool held);
};
