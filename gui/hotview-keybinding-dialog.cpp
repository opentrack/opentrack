#include "hotview-keybinding-dialog.hpp"

#include "listener.h"

#include <QCoreApplication>
#include <QKeySequence>

bool prompt_hotview_key_binding(QWidget* parent, hotview_key& out)
{
    bool accepted = false;

    keyboard_listener listener(parent);
    listener.setWindowModality(Qt::ApplicationModal);
    listener.setWindowTitle(QCoreApplication::translate("hotview_keybinding_dialog", "Press Snap View shortcut"));

    QObject lifetime;

    QObject::connect(&listener, &keyboard_listener::key_pressed,
                     &lifetime,
                     [&](const QKeySequence& seq)
                     {
                         const QString key = seq.toString(QKeySequence::PortableText);

                         for (const QChar& ch : key)
                         {
                             if (!ch.isPrint())
                             {
                                 listener.close();
                                 return;
                             }
                         }

                         out.keycode = key;
                         out.guid.clear();
                         out.button = -1;
                         accepted = !out.keycode.isEmpty();
                         listener.close();
                     });

    QObject::connect(&listener, &keyboard_listener::joystick_button_pressed,
                     &lifetime,
                     [&](const QString& guid, int button, bool held)
                     {
                         if (held)
                             return;

                         out.keycode.clear();
                         out.guid = guid;
                         out.button = button;
                         accepted = !out.guid.isEmpty();
                         listener.close();
                     });

    listener.exec();
    return accepted;
}
