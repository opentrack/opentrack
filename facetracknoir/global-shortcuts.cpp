#include "facetracknoir/facetracknoir.h"

#if defined(_WIN32)
#   ifndef DIRECTINPUT_VERSION
#       define DIRECTINPUT_VERSION 0x800
#   endif
#   include <windows.h>
#   include <dinput.h>

QList<int> global_windows_key_sequences =
    QList<int>()
    << 0
    << DIK_A
    << DIK_B
    << DIK_C
    << DIK_D
    << DIK_E
    << DIK_F
    << DIK_G
    << DIK_H
    << DIK_I
    << DIK_J
    << DIK_K
    << DIK_L
    << DIK_M
    << DIK_N
    << DIK_O
    << DIK_P
    << DIK_Q
    << DIK_R
    << DIK_S
    << DIK_T
    << DIK_U
    << DIK_V
    << DIK_W
    << DIK_X
    << DIK_Y
    << DIK_Z
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
    << DIK_1
    << DIK_2
    << DIK_3
    << DIK_4
    << DIK_5
    << DIK_6
    << DIK_7
    << DIK_8
    << DIK_9
    << DIK_0
    << DIK_LEFT
    << DIK_RIGHT
    << DIK_UP
    << DIK_DOWN
    << DIK_PGUP
    << DIK_DOWN
    << DIK_HOME
    << DIK_END
    << DIK_BACK
    << DIK_DELETE
    << DIK_RETURN;
#endif

QList<QString> global_key_sequences =
    QList<QString>()
    << ""
    << "A"
    << "B"
    << "C"
    << "D"
    << "E"
    << "F"
    << "G"
    << "H"
    << "I"
    << "J"
    << "K"
    << "L"
    << "M"
    << "N"
    << "O"
    << "P"
    << "Q"
    << "R"
    << "S"
    << "T"
    << "U"
    << "V"
    << "W"
    << "X"
    << "Y"
    << "Z"
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
    << "1"
    << "2"
    << "3"
    << "4"
    << "5"
    << "6"
    << "7"
    << "8"
    << "9"
    << "0"
    << "Left"
    << "Right"
    << "Up"
    << "Down"
    << "PgUp"
    << "PgDown"
    << "Home"
    << "End"
    << "Backspace"
    << "Del"
    << "Enter"
;


