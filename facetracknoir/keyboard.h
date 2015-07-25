#pragma once
#include "ui_keyboard_listener.h"
#include <QLabel>
#include <QKeyEvent>

class KeyboardListener : public QLabel
{
    Q_OBJECT
    Ui_keyboard_listener ui;
public:
    KeyboardListener(QWidget* parent = nullptr) : QLabel(parent)
    {
        ui.setupUi(this);
        setFocusPolicy(Qt::StrongFocus);
    }
    void keyPressEvent(QKeyEvent* event) override
    {
        {
            switch (event->key() | event->modifiers())
            {
            case 83886113: // ctrl
            case 50331680: // shift
            case 150994979: // alt
            case 16777250: // meta
                return;
            default: break;
            }
        }
        emit key_pressed(QKeySequence(event->key()));
    }
signals:
    void key_pressed(QKeySequence k);
};
