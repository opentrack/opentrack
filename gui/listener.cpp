#undef NDEBUG
#include <cassert>

#include "listener.h"

#include <QDebug>

#ifdef _WIN32
#include "input/win32-shortcuts.h"

void keyboard_listener::receive_key(const Key& k)
{
    if(!k.guid.isEmpty())
    {
        int mods = 0;
        if (k.alt) mods |= Qt::AltModifier;
        if (k.shift) mods |= Qt::ShiftModifier;
        if (k.ctrl) mods |= Qt::ControlModifier;

        emit joystick_button_pressed(k.guid, k.keycode | mods, k.held);
    }
    else
    {
        Qt::KeyboardModifiers m;
        QKeySequence k_;

        if (win_key::dik_to_qt(k, k_, m))
            for (unsigned i = 0; i < (unsigned)k_.count(); i++)
                emit key_pressed(QKeySequence(int(m) | k_[i].toCombined()));
    }
}

#endif

keyboard_listener::keyboard_listener(QWidget* parent) : QDialog(parent)
{
    ui.setupUi(this);
    setFocusPolicy(Qt::StrongFocus);
#ifdef _WIN32
    (void)token;
#endif
}

#if !defined _WIN32
void keyboard_listener::keyPressEvent(QKeyEvent* event)
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
