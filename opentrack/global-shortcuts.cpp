#include <QList>
#include <QString>

extern QList<QString> global_key_sequences;
extern QList<int> global_windows_key_sequences;

#if defined(_WIN32)
#   ifndef DIRECTINPUT_VERSION
#       define DIRECTINPUT_VERSION 0x800
#   endif
#   include <windows.h>
#   include <dinput.h>

QList<int> global_windows_key_sequences =
    QList<int>()
    << 0
    << DIK_F1
    << DIK_F2
    << DIK_F3
    << DIK_F4
    << DIK_F5
    << DIK_F6
    << DIK_F7
    << DIK_F8
    << DIK_F9
    << DIK_F10
    << DIK_F11
    << DIK_F12
    << DIK_LEFT
    << DIK_RIGHT
    << DIK_UP
    << DIK_DOWN
    << DIK_PGUP
    << DIK_PGDN
    << DIK_HOME
    << DIK_END
    << DIK_BACK
    << DIK_DELETE
    << DIK_RETURN;
#endif

QList<QString> global_key_sequences =
    QList<QString>()
    << ""
    << "F1"
    << "F2"
    << "F3"
    << "F4"
    << "F5"
    << "F6"
    << "F7"
    << "F8"
    << "F9"
    << "F10"
    << "F11"
    << "F12"
    << "Left"
    << "Right"
    << "Up"
    << "Down"
    << "PgUp"
    << "PgDown"
    << "Home"
    << "End"
    << "Del"
;


