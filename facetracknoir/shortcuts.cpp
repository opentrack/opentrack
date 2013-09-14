#if defined(__WIN32) || defined(_WIN32)
#include "facetracknoir/facetracknoir.h"
#include "facetracknoir/shortcut-dialog.h"
#include <windows.h>

KeybindingWorkerDummy::~KeybindingWorkerDummy() {
    if (dinkeyboard) {
        dinkeyboard->Unacquire();
        dinkeyboard->Release();
    }
    if (din)
        din->Release();
}

KeybindingWorkerDummy::KeybindingWorkerDummy(FaceTrackNoIR& w, Key keyCenter)
: kCenter(keyCenter), window(w), should_quit(true), din(0), dinkeyboard(0)
{
    if (DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&din, NULL) != DI_OK) {
        qDebug() << "setup DirectInput8 Creation failed!" << GetLastError();
        return;
    }
    if (din->CreateDevice(GUID_SysKeyboard, &dinkeyboard, NULL) != DI_OK) {
        din->Release();
        din = 0;
        qDebug() << "setup CreateDevice function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->SetDataFormat(&c_dfDIKeyboard) != DI_OK) {
        qDebug() << "setup SetDataFormat function failed!" << GetLastError();
        dinkeyboard->Release();
        dinkeyboard = 0;
        din->Release();
        din = 0;
        return;
    }

    if (dinkeyboard->SetCooperativeLevel(window.winId(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND) != DI_OK) {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup SetCooperativeLevel function failed!" << GetLastError();
        return;
    }
    if (dinkeyboard->Acquire() != DI_OK)
    {
        dinkeyboard->Release();
        din->Release();
        din = 0;
        dinkeyboard = 0;
        qDebug() << "setup dinkeyboard Acquire failed!" << GetLastError();
        return;
    }
    should_quit = false;
}

#define PROCESS_KEY(k, s) \
    if (isKeyPressed(&k, keystate) && (!k.ever_pressed ? (k.timer.start(), k.ever_pressed = true) : k.timer.restart() > 100)) \
        window.s();

static bool isKeyPressed( const Key *key, const BYTE *keystate ) {
    bool shift;
    bool ctrl;
    bool alt;

    if (keystate[key->keycode] & 0x80) {
        shift = ( (keystate[DIK_LSHIFT] & 0x80) || (keystate[DIK_RSHIFT] & 0x80) );
        ctrl  = ( (keystate[DIK_LCONTROL] & 0x80) || (keystate[DIK_RCONTROL] & 0x80) );
        alt   = ( (keystate[DIK_LALT] & 0x80) || (keystate[DIK_RALT] & 0x80) );

        //
        // If one of the modifiers is needed and not pressed, return false.
        //
        if (key->shift && !shift) return false;
        if (key->ctrl && !ctrl) return false;
        if (key->alt && !alt) return false;

        //
        // All is well!
        //
        return true;
    }
    return false;
}

void KeybindingWorkerDummy::run() {
    BYTE keystate[256];
    while (!should_quit)
    {
        if (dinkeyboard->GetDeviceState(256, (LPVOID)keystate) != DI_OK) {
            qDebug() << "Tracker::run GetDeviceState function failed!" << GetLastError();
            Sleep(25);
            continue;
        }

        PROCESS_KEY(kCenter, shortcutRecentered);

        Sleep(25);
    }
}
#else
#endif
