#pragma once
#include "ui_keyboard_listener.h"
#ifdef _WIN32
#include "logic/win32-shortcuts.h"
#include "dinput/keybinding-worker.hpp"
#endif
#include <QLabel>
#include <QKeyEvent>
#include <QDebug>

class KeyboardListener : public QLabel
{
    Q_OBJECT
    Ui_keyboard_listener ui;
#ifdef _WIN32
    KeybindingWorker::Token token;
#endif
public:
    KeyboardListener(QWidget* parent = nullptr) : QLabel(parent)
#ifdef _WIN32
      , token([&](const Key& k) {
        if(k.guid != "")
        {
            int mods = 0;
            if (k.alt) mods |= Qt::AltModifier;
            if (k.shift) mods |= Qt::ShiftModifier;
            if (k.ctrl) mods |= Qt::ControlModifier;
            joystick_button_pressed(k.guid, k.keycode | mods, k.held);
        }
        else
        {
            Qt::KeyboardModifiers m;
            QKeySequence k_;
            if (win_key::to_qt(k, k_, m))
                key_pressed(static_cast<QVariant>(k_).toInt() | m);
        }
    })
#endif
    {
        ui.setupUi(this);
        setFocusPolicy(Qt::StrongFocus);
    }
#ifndef _WIN32
    void keyPressEvent(QKeyEvent* event) override
    {
        //qDebug() << "k" << (event->key() | event->modifiers());
        switch (event->key() | event->modifiers())
        {
        default:
            emit key_pressed(QKeySequence(event->key() | event->modifiers()));
            break;
        }
    }
#endif
signals:
    void key_pressed(QKeySequence k);
    void joystick_button_pressed(QString guid, int idx, bool held);
};
