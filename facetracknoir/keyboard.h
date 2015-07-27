#pragma once
#include "ui_keyboard_listener.h"
#include <QLabel>
#include <QKeyEvent>
#include <QDebug>

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
        //qDebug() << "k" << (event->key() | event->modifiers());
        switch (event->key() | event->modifiers())
        {
        case 83886113: // ctrl
        case 50331680: // shift
        case 150994979: // alt
        case 218103841: // ctrl+alt
        case 218103843: // ctrl+alt
        case 117440545: // ctrl+shift
        case 117440544: // ctrl+shift
        case 184549408: // alt+shift
        case 184549411: // alt+shift
        case 251658272: // ctrl+alt+shift
        case 251658275: // ctrl+alt+shift
        case 251658273: // ctrl+alt+shift
            break;
        default:
            emit key_pressed(QKeySequence(event->key() | event->modifiers()));
            break;
        }
    }
signals:
    void key_pressed(QKeySequence k);
};
