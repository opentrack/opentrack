#include "keyboard.h"

#include <QDebug>

KeyboardListener::KeyboardListener(QWidget* parent) :
    QDialog(parent)
#if defined _WIN32
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
// token initializer ends, real ctor body begins
#endif
{
    ui.setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
}

#if !defined _WIN32
void KeyboardListener::keyPressEvent(QKeyEvent* event)
{
    switch (event->key())
    {
    case Qt::Key_Control:
    case Qt::Key_Shift:
    case Qt::Key_Meta:
    case Qt::Key_Alt:
    case Qt::Key_AltGr:
    case Qt::Key_NumLock:
        break;
    case Qt::Key_Escape:
        close();
        break;

    default:
        emit key_pressed(QKeySequence(event->key() | event->modifiers()));
        break;
    }
}
#endif
